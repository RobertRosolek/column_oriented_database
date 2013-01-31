#ifndef PROJECT1_SCANEXECUTER_H
#define PROJECT1_SCANEXECUTER_H

#include <vector>
#include <utility>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"

class ScanExecuter {
	private:
		const ScanOperation &query;
		Server *server;

		std::vector<Type> columnTypes;

	public:	
		ScanExecuter(const ScanOperation &query, Server *server);

		// returns if there is any data left to pull
		void pull(const size_t blockSizeRequested, size_t &blockSizeReceived, std::vector<void*> &columns);
};


#endif // PROJECT1_SCANEXECUTER_H
