#include <vector>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <functional>
#include <boost/functional/hash.hpp>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "GroupByExecuter.h"
#include "OperationExecuter.h"
#include "OperationTypeVisitor.h"
#include "ExecuterConstants.h"
#include "Debugger.h"

static const size_t INITIAL_RESULT_BLOCK_SIZE = BLOCK_LEN;

// when we run out of memory this function
// shows how many rows now to allocate
static size_t nextBlockSize(size_t blockSize) {
	// + BLOC_LEN ensures that we will be able 
	// to add at least one new block after reallocation
	return (3 * blockSize) / 2 + BLOCK_LEN;
}

// hash set used to get standard double hash functions
static std::unordered_set<double> doubleHashTable;

// hash function copied from
// http://www.concentric.net/~Ttwang/tech/inthash.htm
static inline unsigned int hashUnsigned( unsigned int a) {
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}


struct HashFunction {
	Key k;
	HashFunction(Key k) : k(k) {}
	inline long operator() (void *column, Type type) {
		switch (type) {
	
			case INT:
				return hashUnsigned( *((int*) column + k) ); 

			case DOUBLE:
				return doubleHashTable.hash_function()(*((double*) column + k));

			default:
				return *((bool*) column + k); // identity hash function for bools
		}
	}
};

std::vector<Type>* KeyHash::groupedByColumnTypes = NULL;
std::vector<void*>* KeyHash::groupedByResultColumns = NULL;
std::vector<long> KeyHash::columnHashValues = std::vector<long>();

long KeyHash::operator()(const Key &k) const {

	// calculate hash function for each column first
	HashFunction hashFunction(k);
	std::transform(this->groupedByResultColumns->begin(), this->groupedByResultColumns->end(), 
			this->groupedByColumnTypes->begin(), this->columnHashValues.begin(), hashFunction);

	// combine hash values to obtain one hash value for the whole row
	long res = boost::hash_range(this->columnHashValues.begin(), this->columnHashValues.end());

	return res;
}

std::vector<Type>* KeyEq::groupedByColumnTypes = NULL;
std::vector<void*>* KeyEq::groupedByResultColumns = NULL;

template<typename T>
static inline bool compareData(void* s1, size_t offset1, void *s2, size_t offset2) {
	return * ( (T*) s1 + offset1) == * ( (T*) s2 + offset2);
}

bool KeyEq::operator()(const Key &k1, const Key &k2) const {
		
	auto typeIterator = KeyEq::groupedByColumnTypes->begin();
	auto columnIterator = KeyEq::groupedByResultColumns->begin();

	for (; typeIterator != KeyEq::groupedByColumnTypes->end(); ++typeIterator, ++columnIterator) 
		switch (*typeIterator) {
			case INT:
				if (!compareData<int>( *columnIterator, k1, *columnIterator, k2))
					return false;
				break;
			case DOUBLE:
				if (!compareData<double>( *columnIterator, k1, *columnIterator, k2))
					return false;
				break;
			default:
				if (!compareData<bool>( *columnIterator, k1, *columnIterator, k2))
					return false;
				break;
		}

	return true;
}

GroupByExecuter::GroupByExecuter(const GroupByOperation &query, Server *server) : 
	query(query), server(server), 
	sourceExecuter(OperationExecuter(query.source(), server)), 
	finishedGrouping(false), areResultColumnsDeallocated(false),
   	resultBlockSize(INITIAL_RESULT_BLOCK_SIZE), nextRow(0), nextPullRow(0) {
		
	// get the source types
	OperationTypeVisitor::getType(query.source(), this->sourceColumnTypes);

	// allocate memory for source blocks
	this->sourceColumns.resize(this->sourceColumnTypes.size());
	allocFullBlocs(this->sourceColumns, this->sourceColumnTypes);

	// get the result types
	OperationTypeVisitor::getGroupedByTypes(query, this->sourceColumnTypes, this->groupedByColumnTypes);
	OperationTypeVisitor::getAggregationsTypes(query, this->sourceColumnTypes, this->aggregationColumnTypes);

	// allocate memory for result Columns
	this->groupedByResultColumns.resize(this->groupedByColumnTypes.size());
	this->aggregationResultColumns.resize(this->aggregationColumnTypes.size());
	assert(INITIAL_RESULT_BLOCK_SIZE == BLOCK_LEN);
	allocFullBlocs(this->groupedByResultColumns, this->groupedByColumnTypes);
	allocFullBlocs(this->aggregationResultColumns, this->aggregationColumnTypes);
}

GroupByExecuter::~GroupByExecuter() {
	assert(this->areResultColumnsDeallocated);	
}

void GroupByExecuter::deallocateResultColumns() {
	std::for_each(this->aggregationResultColumns.begin(), this->aggregationResultColumns.end(), free);
	std::for_each(this->groupedByResultColumns.begin(), this->groupedByResultColumns.end(), free);
	this->areResultColumnsDeallocated = true;
}

template<typename T>
static inline void copyBlockTemplate(void *source, size_t sourceOffset, void* dest, size_t destOffset, size_t len) {
	std::copy( (T*) source + sourceOffset, (T*) source + sourceOffset + len, (T*) dest + destOffset);
}

static inline void copyBlock(void *source, size_t sourceOffset, void *dest, size_t destOffset, Type type, size_t len) {
	switch (type) {
		case INT:
			copyBlockTemplate<int>(source, sourceOffset, dest, destOffset, len);
			break;

		case DOUBLE:
			copyBlockTemplate<double>(source, sourceOffset, dest, destOffset, len);
			break;

		default:
			copyBlockTemplate<bool>(source, sourceOffset, dest, destOffset, len);
	}
}	

template<typename T>
static inline void copyDataTemplate(void *source, size_t sourceOffset, void *dest, size_t destOffset) {
	T *p = (T*) dest + destOffset;
	*p = * ( (T*) source + sourceOffset);
}

// essentially copyBlock with len == 1 but should be more efficient
static inline void copyData(void *source, size_t sourceOffset, void *dest, size_t destOffset, Type type) {
	switch (type) {
		case INT:
			copyDataTemplate<int>(source, sourceOffset, dest, destOffset);
			break;

		case DOUBLE:
			copyDataTemplate<double>(source, sourceOffset, dest, destOffset);
			break;

		default:
			copyDataTemplate<bool>(source, sourceOffset, dest, destOffset);
	}
}

template<typename T>
static inline void setZeroTemplate(void *dest, size_t offset) {
	T *p = (T*) dest + offset;
	*p = 0;
}

static inline void setZero(void *dest, size_t offset, Type type) {
	switch (type) {
		case INT:
			setZeroTemplate<int>(dest, offset);
			break;

		case DOUBLE:
			setZeroTemplate<double>(dest, offset);
			break;

		default:
			setZeroTemplate<bool>(dest, offset);
	}
}

template<typename T>
static inline void addToTemplate(void *source, size_t sourceOffset, void *dest, size_t destOffset) {
	T *p = (T*) dest + destOffset;
	*p = *p + *( (T*) source + sourceOffset);
}

static inline void addTo(void *source, size_t sourceOffset, void* dest, size_t destOffset, Type type) {
	switch (type) {
		case INT:
			addToTemplate<int>(source, sourceOffset, dest, destOffset);
			break;
		case DOUBLE:
			addToTemplate<double>(source, sourceOffset, dest, destOffset);
			break;
		default:
			assert(false && "bools should not be summed");
	}
}

static inline void inc(void *dest, size_t offset) {
	int *p = (int*) dest + offset;
	*p = *p + 1;
}

void GroupByExecuter::reallocateResultColumns() {
	// determine new result block size
	size_t newBlockSize = nextBlockSize(this->resultBlockSize);

	// allocate new columns	
	std::vector<void*> newGroupedByResultColumns = std::vector<void*>(this->groupedByResultColumns.size());
	std::vector<void*> newAggregationResultColumns = std::vector<void*>(this->aggregationResultColumns.size());
	BlockAllocator allocBlock;
	std::transform(this->groupedByColumnTypes.begin(), this->groupedByColumnTypes.end(), 
			newGroupedByResultColumns.begin(), std::bind1st(allocBlock, newBlockSize));
	std::transform(this->aggregationColumnTypes.begin(), this->aggregationColumnTypes.end(),
			newAggregationResultColumns.begin(), std::bind1st(allocBlock, newBlockSize));

	// copy the values to new result block
	for (size_t i = 0, noOfColumns = this->groupedByResultColumns.size(); i < noOfColumns; ++i) 
		copyBlock(this->groupedByResultColumns[i], 0,
				newGroupedByResultColumns[i], 0, this->groupedByColumnTypes[i], this->nextRow);
	for (size_t i = 0, noOfColumns = this->aggregationResultColumns.size(); i < noOfColumns; ++i)
		copyBlock(this->aggregationResultColumns[i], 0,
				newAggregationResultColumns[i], 0, this->aggregationColumnTypes[i], this->nextRow);
	
	// deallocate old result columns
	std::for_each(this->groupedByResultColumns.begin(), this->groupedByResultColumns.end(), free);
	std::for_each(this->aggregationResultColumns.begin(), this->aggregationResultColumns.end(), free);

	// set new columns as current result columns
	std::swap(this->groupedByResultColumns, newGroupedByResultColumns);
	std::swap(this->aggregationResultColumns, newAggregationResultColumns);

	// set new result block size
	this->resultBlockSize = newBlockSize;
}

// add row with given number from source columns to
// data structure 
void GroupByExecuter::addNextRow(size_t sourceRow, HashTable &hashTable) {
	
	for (int i = 0, noOfColumns = (int) this->groupedByResultColumns.size(); i < noOfColumns; ++i) 
		copyData( this->sourceColumns[this->query.group_by_column(i)], sourceRow,
			   this->groupedByResultColumns[i], this->nextRow, this->groupedByColumnTypes[i]);
	// init aggregated values to 0, this is only correct because we have only count and sum aggregations
	for (int i = 0, noOfColumns = (int) this->aggregationResultColumns.size(); i < noOfColumns; ++i) 
		setZero(this->aggregationResultColumns[i], this->nextRow, this->aggregationColumnTypes[i]);


	Key key = this->nextRow;

	// row in result columns to which we will accumulate currently added source Row
	size_t aggregateRow;

	HashTable::iterator it = hashTable.find(key);

	if (it == hashTable.end()) {
		aggregateRow = this->nextRow;
		++this->nextRow;
		hashTable.insert(key);
	}
	else
		aggregateRow = *it;

	// aggregate sourceRow to given result/aggregate row
	for (int i = 0, noOfColumns = (int) this->aggregationResultColumns.size(); i < noOfColumns; ++i) 
		if (this->query.aggregations(i).type() == Aggregation_Type_COUNT) {
			assert(this->aggregationColumnTypes[i] == INT);
			inc(this->aggregationResultColumns[i], aggregateRow);
		}	
		else 
			addTo(this->sourceColumns[this->query.aggregations(i).aggregated_column()], sourceRow, 
					this->aggregationResultColumns[i], aggregateRow, 
					this->aggregationColumnTypes[i]);
	
}

void GroupByExecuter::group(const size_t blockSize) {
	
	size_t blockSizeReceived;

	HashTable hashTable;

	do {

		// allocate place for new rows if needed
		if (this->nextRow + blockSize > this->resultBlockSize)
			this->reallocateResultColumns();

		// get next portion of data from source
		this->sourceExecuter.pull(blockSize, blockSizeReceived, this->sourceColumns);

		// init hash function
		KeyHash::groupedByColumnTypes = & (this->groupedByColumnTypes);
		KeyHash::groupedByResultColumns = & (this->groupedByResultColumns);
		KeyHash::columnHashValues.resize(this->groupedByResultColumns.size());

		// init eq function
		KeyEq::groupedByColumnTypes = & (this->groupedByColumnTypes);
		KeyEq::groupedByResultColumns = & (this->groupedByResultColumns);

		// for each row add it to the data structure
		for (size_t row = 0; row < (size_t) blockSizeReceived; ++row) {	
			assert(this->nextRow < this->resultBlockSize);
			this->addNextRow(row, hashTable);
		}

	} while (blockSizeReceived > 0);

	finishedGrouping = true;
}

void GroupByExecuter::pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns) {

	if (!finishedGrouping) 
		this->group(BLOCK_LEN);

	blockSizeReceived = std::min(blockSizeRequested, this->nextRow - this->nextPullRow);

	size_t noOfGroupedByColumns = this->groupedByResultColumns.size(),
		   noOfAggregationColumns = this->aggregationResultColumns.size();

	// copy grouped by columns
	for (size_t i = 0; i < noOfGroupedByColumns; ++i)
		copyBlock(this->groupedByResultColumns[i], this->nextPullRow, 
				columns[i], 0, this->groupedByColumnTypes[i], blockSizeReceived);

	// copy aggregated columns
	for (size_t i = 0; i < noOfAggregationColumns; ++i)
		copyBlock(this->aggregationResultColumns[i], this->nextPullRow,
				columns[noOfGroupedByColumns + i], 0, this->aggregationColumnTypes[i], blockSizeReceived);

	this->nextPullRow += blockSizeReceived;

	// if done sending data we can free the memory
	if (this->nextPullRow == this->nextRow && !areResultColumnsDeallocated) 
		this->deallocateResultColumns();
}

