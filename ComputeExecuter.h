#ifndef PROJECT1_COMPUTEEXECUTER_H
#define PROJECT1_COMPUTEEXECUTER_H

#include <vector>
#include <utility>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "ExpressionExecuter.h"
#include "OperationExecuter.h"

class ComputeExecuter {
	private:
		const ComputeOperation &query;
		Server *server;

		std::vector<ExpressionExecuter*> expressionExecuters;
		OperationExecuter sourceExecuter;

		std::vector<Type> columnTypes;
		std::vector<Type> sourceColumnTypes;

		std::vector<void*> sourceColumns;

		//void releaseExpressionExecuter(ExpressionExecuter *executer);

	public:	
		ComputeExecuter(const ComputeOperation &query, Server *server);
		~ComputeExecuter();

		// returns if there is any data left to pull
		bool pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns);
};


#endif // PROJECT1_COMPUTEEXECUTER_H
