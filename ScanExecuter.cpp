#include <vector>
#include <cstdlib>
#include <iomanip>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "ScanExecuter.h"
#include "OperationTypeVisitor.h"
#include "Debugger.h"

ScanExecuter::ScanExecuter(const ScanOperation &query, Server *server) : query(query), server(server) {
	OperationTypeVisitor::getType(query, this->columnTypes);
}

void ScanExecuter::pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns) {
	int numberOfColumns = query.column_size();
	
	blockSizeReceived = 0;

	for (int columnNo = 0; columnNo < numberOfColumns; ++columnNo) {

		switch (this->columnTypes[columnNo]) {
			case INT: 
				blockSizeReceived = (size_t) this->server->GetInts(columnNo, (int) blockSizeRequested, (int*) columns[columnNo]);
				break;

			case DOUBLE:
				blockSizeReceived = (size_t) this->server->GetDoubles(columnNo, (int) blockSizeRequested, (double*) columns[columnNo]);
				break;

			case BOOL:
				blockSizeReceived = (size_t) this->server->GetByteBools(columnNo, (int) blockSizeRequested, (bool*) columns[columnNo]);
				break;

			default:
				assert(false && "incorrect scan type");
		} 
	}

	if (debugFlag)
		debugOutputBlock(blockSizeReceived, columns, this->columnTypes);
}

