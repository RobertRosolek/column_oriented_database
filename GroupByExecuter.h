#ifndef PROJECT1_GROUPBYEXECUTER_H
#define PROJECT1_GROUPBYEXECUTER_H

#include <vector>
#include <utility>
#include <unordered_set>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "ExpressionExecuter.h"
#include "OperationExecuter.h"

typedef size_t Key;

struct KeyHash {
	static std::vector<Type> *groupedByColumnTypes;
	static std::vector<void*> *groupedByResultColumns;
	static std::vector<long> columnHashValues;
	long operator() (const Key &k) const;
};

struct KeyEq {
	static std::vector<Type> *groupedByColumnTypes;
	static std::vector<void*> *groupedByResultColumns;
	bool operator() (const Key &k1, const Key &k2) const;
};

class GroupByExecuter {
	private:
		const GroupByOperation &query;
		Server *server;

		OperationExecuter sourceExecuter;

		std::vector<void*> sourceColumns;

		std::vector<Type> sourceColumnTypes;

		std::vector<Type> groupedByColumnTypes;
		std::vector<Type> aggregationColumnTypes;

		bool finishedGrouping;

		typedef std::unordered_set<Key, KeyHash, KeyEq> HashTable;

		// place to store result columns
		// the rows of these columns are the keys in the hash table
		// we allocate the place for this columns in
		// the vector similar way (we do not in advance 
		// know how many columns there will be), 
		// blockSize is the current size
		bool areResultColumnsDeallocated; 
		std::vector<void*> groupedByResultColumns;
		std::vector<void*> aggregationResultColumns;
	   	size_t resultBlockSize;

		void deallocateResultColumns();

		// the first free row in resultColumns
		size_t nextRow;

		// next row to return to pull operation
		size_t nextPullRow;

		void group(const size_t blockSize);

		void reallocateResultColumns();

		void addNextRow(size_t row, HashTable &hashTable);

	public:	
		GroupByExecuter(const GroupByOperation &query, Server *server);
		~GroupByExecuter();

		void pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns);
};


#endif // PROJECT1_GROUPBYEXECUTER_H
