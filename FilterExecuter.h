#ifndef PROJECT1_FILTEREXECUTER_H
#define PROJECT1_FILTEREXECUTER_H

#include <vector>
#include <utility>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "ExpressionExecuter.h"
#include "OperationExecuter.h"

class FilterExecuter {
	private:
		const FilterOperation &query;
		Server *server;

		ExpressionExecuter *expressionExecuter;
		OperationExecuter sourceExecuter;

		void *expressionColumn;

		// those are the column types of both the source and the result
		std::vector<Type> columnTypes;

	public:	
		FilterExecuter(const FilterOperation &query, Server *server);
		~FilterExecuter();

		void pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns);
};


#endif // PROJECT1_FILTEREXECUTER_H
