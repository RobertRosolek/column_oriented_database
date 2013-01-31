OBJS = server.o operations.o OperationExecuter.o ScanExecuter.o OperationTypeVisitor.o ExpressionTypeVisitor.o ComputeExecuter.o \
	   ExpressionExecuter.o FilterExecuter.o GroupByExecuter.o
NUMBERS = 1 2 3 4 5 6 7 8 9 10 11 12
CC = g++
DEBUG += $(DEBUGFLAG)
VECTORIZE = -msse
CFLAGS = -Wall -O3  -DNDEBUG -ffast-math $(VECTORIZE) -march=native -Wextra -Wcast-align -Wconversion -std=c++0x -c $(DEBUG)
LFLAGS = -Wall -O3  -DNDEBUG -march=native -Wextra -Wcast-align -Wconversion $(DEBUG)
PROTOENCODE = protoc operations.proto --encode Operation  # encode the test cases into binary files comman
OPERATIONSPROTO = protoc operations.proto --cpp_out=.

exec_plan : $(OBJS) exec_plan.cpp
	$(CC) $(LFLAGS) exec_plan.cpp $(OBJS) -o exec_plan -l protobuf -l pthread

server.o : server.h server.cc
	$(CC) -march=native -O3 -c server.cc -o server.o $(DEBUG)

operations.o : operations.pb.cc 
	$(CC) -Wall -Wextra -march=native -O3 -c operations.pb.cc -o operations.o

operations.pb.cc : operations.proto
	$(OPERATIONSPROTO)

OperationExecuter.o : OperationExecuter.h OperationExecuter.cpp operations.pb.h ExecuterConstants.h \
	server.h Types.h ScanExecuter.h ComputeExecuter.h OperationTypeVisitor.h FilterExecuter.h GroupByExecuter.h
	$(CC) $(CFLAGS) OperationExecuter.cpp 

ScanExecuter.o : ScanExecuter.h ScanExecuter.cpp operations.pb.h ExecuterConstants.h server.h Types.h OperationTypeVisitor.h
	$(CC) $(CFLAGS) ScanExecuter.cpp

ComputeExecuter.o : ComputeExecuter.h ComputeExecuter.cpp operations.pb.h ExecuterConstants.h server.h Types.h OperationTypeVisitor.h \
	ExpressionExecuter.h
	$(CC) $(CFLAGS) ComputeExecuter.cpp

FilterExecuter.o : FilterExecuter.h FilterExecuter.cpp operations.pb.h ExecuterConstants.h server.h Types.h OperationTypeVisitor.h \
	ExpressionExecuter.h OperationExecuter.h Debugger.h
	$(CC) $(CFLAGS) FilterExecuter.cpp

GroupByExecuter.o : GroupByExecuter.h GroupByExecuter.cpp OperationTypeVisitor.h server.h Types.h ExecuterConstants.h operations.pb.h \
	OperationExecuter.h Debugger.h
	$(CC) $(CFLAGS) GroupByExecuter.cpp

ExpressionExecuter.o : ExpressionExecuter.h ExpressionExecuter.cpp operations.pb.h Types.h Debugger.h ExecuterConstants.h
	$(CC) $(CFLAGS) ExpressionExecuter.cpp

OperationTypeVisitor.o : OperationTypeVisitor.h OperationTypeVisitor.cpp ExpressionTypeVisitor.h
	$(CC) $(CFLAGS) OperationTypeVisitor.cpp

ExpressionTypeVisitor.o : ExpressionTypeVisitor.h ExpressionTypeVisitor.cpp
	$(CC) $(CFLAGS) ExpressionTypeVisitor.cpp

clean:
	\rm -f *.o *~ exec_plan queries/*.pb  *.pb.* a.out

compile_test:
	$(foreach i,$(NUMBERS), $(PROTOENCODE) <queries/q$(i).ascii >queries/q$(i).pb;)

tar:
	tar cfv rr277585.tar makefile operations.proto *.cpp *.cc *.h *.sh 
