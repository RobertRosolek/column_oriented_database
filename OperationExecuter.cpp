#include <iostream>
#include <cassert>

#include "OperationExecuter.h"
#include "ExecuterConstants.h"
#include "ScanExecuter.h"
#include "ComputeExecuter.h"
#include "FilterExecuter.h"
#include "GroupByExecuter.h"
#include "OperationTypeVisitor.h"
#include "Types.h"

OperationExecuter::OperationExecuter(const Operation &query, Server *server) : query(query), server(server) {
	OperationTypeVisitor::getType(query, this->columnTypes);

	if (this->query.has_scan())
		this->scanExecuter = new ScanExecuter(this->query.scan(), this->server);
	else if (this->query.has_compute())
		this->computeExecuter = new ComputeExecuter(this->query.compute(), this->server);
	else if (this->query.has_filter())
		this->filterExecuter = new FilterExecuter(this->query.filter(), this->server);
	else if (this->query.has_group_by())
		this->groupByExecuter = new GroupByExecuter(this->query.group_by(), this->server);
}

OperationExecuter::~OperationExecuter() {
	if (this->query.has_scan())
		delete this->scanExecuter;
	else if (this->query.has_compute())
		delete this->computeExecuter;
	else if (this->query.has_filter())
		delete this->filterExecuter;
	else if (this->query.has_group_by())
		delete this->groupByExecuter;
}

void OperationExecuter::executeToServer() {
	size_t blockSizeReceived;
	
	// allocate memory for the result blocks
	std::vector<void*> columns;
	columns.resize(this->columnTypes.size());
	allocFullBlocs(columns, this->columnTypes);

	do {
			
		// pull a block of data
		OperationExecuter::pull(BLOCK_LEN, blockSizeReceived, columns);

		// send block to the server	and free the memory
		for (int columnNo = 0, s = (int) columns.size(); columnNo < s; ++columnNo) {
				switch (this->columnTypes[columnNo]) {

					case INT:		this->server->ConsumeInts(columnNo,(int) blockSizeReceived, (int *) columns[columnNo]);
									break;

					case DOUBLE:	this->server->ConsumeDoubles(columnNo,(int) blockSizeReceived, (double *) columns[columnNo]);
									break;

					case BOOL:		this->server->ConsumeByteBools(columnNo,(int) blockSizeReceived, (bool *) columns[columnNo]);
									break;

					default: 		assert(false && "wrong data type");
				}		
		}
	} 
	while (blockSizeReceived > 0);

	// deallocate memory
	std::for_each(columns.begin(), columns.end(), free);
}

void OperationExecuter::pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns) {

	if (this->query.has_scan()) 
		this->scanExecuter->pull(blockSizeRequested, blockSizeReceived, columns);
	else if (this->query.has_compute()) 
		this->computeExecuter->pull(blockSizeRequested, blockSizeReceived, columns);
	else if (this->query.has_filter()) 
		this->filterExecuter->pull(blockSizeRequested, blockSizeReceived, columns);
	else if (this->query.has_group_by()) 
		this->groupByExecuter->pull(blockSizeRequested, blockSizeReceived, columns);
	else 
		assert(false && "incorrect operation - it should be exactly one of : scan, compute, filter or group");	
}
