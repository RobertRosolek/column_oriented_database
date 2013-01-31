#include <vector>
#include <cstdlib>
#include <iostream>
#include <numeric>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "FilterExecuter.h"
#include "OperationExecuter.h"
#include "OperationTypeVisitor.h"
#include "ExpressionExecuter.h"
#include "ExecuterConstants.h"

FilterExecuter::FilterExecuter(const FilterOperation &query, Server *server) : 
	query(query), server(server), 
	sourceExecuter(OperationExecuter(query.source(), server)) {
	
	// get the source (target) types
	OperationTypeVisitor::getType(query, this->columnTypes);

	// allocate and initialise expression executer
	this->expressionExecuter = new ExpressionExecuter(query.expression(), this->columnTypes);

	// allocate full block of bools
	expressionColumn = allocFullBlock(BOOL);
}

FilterExecuter::~FilterExecuter() {
	delete this->expressionExecuter;

	free(expressionColumn);
}

template<typename T>
static T* filter(bool* filterBitMap, T* first, T* last) {
	T* result = first;
	for (; first != last; ++first, ++filterBitMap)
		if (*filterBitMap)
			*result++ = *first;
	return result;
}	

void FilterExecuter::pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns) {

	size_t blockSize = 0;

	int noOfColumns = (int) columns.size();
	// we will fill the columns in prospectively many
	// iterations, this shows the first free position
	// to copy new data in corresponding column
	std::vector<void*> nextPositions = columns;
	do {
		// pull the data from the source
		// we request as much data as is needed to make the block full
		this->sourceExecuter.pull(blockSizeRequested - blockSize, blockSizeReceived, nextPositions);
		
		// evaluate filter expression
		this->expressionExecuter->eval(blockSizeReceived, nextPositions, this->expressionColumn);
			
		size_t newBlockSize = std::accumulate( (bool*) this->expressionColumn, (bool*) this->expressionColumn + blockSizeReceived, 0); 
		
		// filtrate the data
		for (int i = 0; i < noOfColumns; ++i) 
			if (this->columnTypes[i] == INT) {
				int *x = filter( (bool*) this->expressionColumn, (int*) nextPositions[i], (int*) nextPositions[i] + blockSizeReceived);
				assert(x - ( (int*) nextPositions[i]) == (int) newBlockSize);
				nextPositions[i] = x;
			}
			else if (this->columnTypes[i] == DOUBLE) {
				double *x = filter( (bool*) this->expressionColumn, (double*) nextPositions[i], (double*) nextPositions[i] + blockSizeReceived);
				assert(x - ( (double*) nextPositions[i]) == (int) newBlockSize);
				nextPositions[i] = x;
			}
			else if (this->columnTypes[i] == BOOL) {
				bool *x = filter ( (bool*) this->expressionColumn, (bool*) nextPositions[i], (bool*) nextPositions[i] + blockSizeReceived);
				assert(x - ( (bool*) nextPositions[i]) == (int) newBlockSize);
				nextPositions[i] = x;
			} 

		blockSize += newBlockSize;
	}
   	// we are done when there is no data left or we have exceeded the necessary fraction of requested block size 	
	while (blockSizeReceived > 0 && blockSize * FILTER_PROPORTION_DENOMINATOR < FILTER_PROPORTION_NUMERATOR * blockSizeRequested);
	
	blockSizeReceived = blockSize;
}

