#include <vector>
#include <cstdlib>
#include <iostream>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "ComputeExecuter.h"
#include "OperationExecuter.h"
#include "OperationTypeVisitor.h"
#include "ExpressionExecuter.h"

ComputeExecuter::ComputeExecuter(const ComputeOperation &query, Server *server) : 
	query(query), server(server), sourceExecuter(OperationExecuter(query.source(), server))  {
	
	// get the source and target types
	OperationTypeVisitor::getType(query, this->columnTypes);
	OperationTypeVisitor::getType(query.source(), this->sourceColumnTypes);

	// allocate memory for source columns
	this->sourceColumns.resize(this->sourceColumnTypes.size());
	allocFullBlocs(this->sourceColumns, this->sourceColumnTypes);

	// allocate and initialise expression executors
	int numberOfExpressions = query.expressions_size();
	this->expressionExecuters.resize(numberOfExpressions);
	
	for (int i = 0; i < numberOfExpressions; ++i) {
		this->expressionExecuters[i] = new ExpressionExecuter(query.expressions(i), this->sourceColumnTypes);
	}
}

inline static void releaseExpressionExecuter(ExpressionExecuter *executer) {
	delete executer;
}

ComputeExecuter::~ComputeExecuter() {

	// deallocate expression executers
	std::for_each(this->expressionExecuters.begin(), this->expressionExecuters.end(), releaseExpressionExecuter);

	// deallocate source columns
	std::for_each(this->sourceColumns.begin(), this->sourceColumns.end(), free);

}

bool ComputeExecuter::pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns) {

	// pull the block of data from the source
	sourceExecuter.pull(blockSizeRequested, blockSizeReceived, this->sourceColumns);

	int numberOfColumns = this->query.expressions_size();

	// evaluate expressions
	for (int i = 0; i < numberOfColumns; ++i) 
		this->expressionExecuters[i]->eval(blockSizeReceived, sourceColumns, columns[i]);	

	return blockSizeReceived != 0;
}

