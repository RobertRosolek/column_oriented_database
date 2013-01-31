#ifndef PROJECT1_OPERATIONEXECUTER_H
#define PROJECT1_OPERATIONEXECUTER_H

#include <vector>
#include <utility>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
/*#include "FilterExecuter.h"
#include "ScanExecuter.h"
#include "ComputeExecuter.h"*/

class FilterExecuter;
class ScanExecuter;
class ComputeExecuter;
class GroupByExecuter;

class OperationExecuter {
	private:
		const Operation &query;
		Server *server;

		std::vector<Type> columnTypes;

		FilterExecuter *filterExecuter;
		ComputeExecuter *computeExecuter;
		ScanExecuter *scanExecuter;
		GroupByExecuter *groupByExecuter;

	public:
		OperationExecuter(const Operation &query, Server *server);
		~OperationExecuter();

		// execute given query and send results to the server
		void executeToServer();

		// returns if there is any data left to pull
		void pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns);
};


#endif // PROJECT1_OPERATIONEXECUTER_H
