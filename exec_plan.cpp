// Copyright 2012 Google Inc. All Rights Reserved.
// Author: onufry@google.com (Onufry Wojtaszczyk)

#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "server.h"
#include "operations.pb.h"
#include "OperationExecuter.h"

int main(int argc, char* argv[]) {

	// exactly two parameters should be passed
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " {query number} {full path to the file with query}" << std::endl;
		return -1;
	}

	// query id and object representing query itself
	int query_id = atoi(argv[1]);
	Operation query;

	// parse the protocol-buffer binary file and create query object
	int file = open(argv[2], O_RDONLY);
	google::protobuf::io::FileInputStream input(file);
	google::protobuf::TextFormat::Parse(&input, &query);
	
	// create server which will serve the data
	Server *server = CreateServer(query_id);

	// execute query
	OperationExecuter executer(query, server);
	executer.executeToServer();

	// delete server - this stops the timer 
	// and triggers correctness check
	delete server;

	return 0;
}
