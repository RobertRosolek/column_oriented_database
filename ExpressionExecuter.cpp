#include <vector>
#include <cstdlib>
#include <functional>
#include <cmath>
#include <iostream>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"
#include "Operations.h"
#include "ExpressionExecuter.h"
#include "ExpressionTypeVisitor.h"
#include "Debugger.h"

ExpressionExecuter::ExpressionExecuter(const Expression &expr, const std::vector<Type> &sourceColumnTypes) : 
	expr(expr), sourceColumnTypes(sourceColumnTypes), resultType(ExpressionTypeVisitor::getType(expr, sourceColumnTypes)) {
		
	int noOfChildren = expr.children_size();

	// allocate and intialise children expression executers
	this->childrenExecuters.resize(noOfChildren);
	for (int i = 0; i < noOfChildren; ++i)
		this->childrenExecuters[i] = new ExpressionExecuter(expr.children(i), this->sourceColumnTypes);	

	this->childrenColumns.resize(noOfChildren);

	this->isAllocatedChildColumn = std::vector<bool>(noOfChildren, false);

	this->childrenTypes.resize(noOfChildren);
	for (int i = 0; i < noOfChildren; ++i)
		this->childrenTypes[i] = ExpressionTypeVisitor::getType(expr.children(i), this->sourceColumnTypes);
}

inline static void releaseExpressionExecuter(ExpressionExecuter *executer) {
	delete executer;
}

inline static void* releaseIfAllocatedChildColumn(void *p, bool isAllocated) {
	if (isAllocated) 
		free(p);
	return p;
}

ExpressionExecuter::~ExpressionExecuter() {

	// deallocate children expression executers
	std::for_each(this->childrenExecuters.begin(), this->childrenExecuters.end(), releaseExpressionExecuter);

	// deallocate (previously allocated) child columns
	std::transform(this->childrenColumns.begin(), this->childrenColumns.end(), 
			this->isAllocatedChildColumn.begin(), this->childrenColumns.begin(), releaseIfAllocatedChildColumn);
}

template<typename T>
inline void ExpressionExecuter::allocateIfNotAllocatedChildColumn(const int index) {
	if (! this->isAllocatedChildColumn[index]) {
		this->childrenColumns[index] = malloc(BLOCK_LEN * sizeof(T));
		this->isAllocatedChildColumn[index] = true;
	}
}

// generic function evaluating if-else expression
template<typename T>
inline void ExpressionExecuter::evalIf(const size_t blockSize, const std::vector<void*> &sourceColumns, void* column) {
	// allocate (if needed) child column for condition expression and evaluate it in its block
	this->allocateIfNotAllocatedChildColumn<bool>(0);
	this->childrenExecuters[0]->eval(blockSize, sourceColumns, this->childrenColumns[0]);
	
	// evaluate left child in result block
	this->childrenExecuters[1]->eval(blockSize, sourceColumns, column);
	
	// allocate (if needed) right child and evaluate it in its block
	this->allocateIfNotAllocatedChildColumn<T>(2);
	this->childrenExecuters[2]->eval(blockSize, sourceColumns, this->childrenColumns[2]);

	bool *a = (bool*) this->childrenColumns[0], *b = a + blockSize;
	T *c = (T*) column, *d = (T*) this->childrenColumns[2];

	// main algorithm performing choosing the proper value
	for (;a != b; ++a, ++c, ++d) 
		if (! *a)
			*c = *d; 
}

// generic function evaluating binary operator expression,
// it evaluates children it their own blocks
// and then performs operations putting the result in this executor block
// this should only be used when T1 <> R and T2 <> R (otherwise inPlace version should be used)
template<typename T1, typename T2, typename R>
inline void ExpressionExecuter::evalBinOp(const size_t blockSize, const std::vector<void*> &sourceColumns,
		void *column, const Expression_Operator &expressionOperator) {

	// allocate (if needed) left child and evaluate it in its block
	this->allocateIfNotAllocatedChildColumn<T1>(0);
	this->childrenExecuters[0]->eval(blockSize, sourceColumns, this->childrenColumns[0]);

	// allocate (if needed) left child and evaluate it in its block
	this->allocateIfNotAllocatedChildColumn<T2>(1);
	this->childrenExecuters[1]->eval(blockSize, sourceColumns, this->childrenColumns[1]);

	T1 *a = (T1*) this->childrenColumns[0], *b = a + blockSize;
	T2 *c = (T2*) this->childrenColumns[1];
	R *d = (R*) column;

	switch (expressionOperator) {

		case Expression_Operator_FLOATING_DIVIDE:
			std::transform(a, b, c, d, std::divides<R>());
			break;

		case Expression_Operator_LOWER:
			std::transform(a, b, c, d, std::less<T1>());
			break;

		case Expression_Operator_GREATER:
			std::transform(a, b, c, d, std::greater<T1>());
			break;

		case Expression_Operator_EQUAL:
			std::transform(a, b, c, d, std::equal_to<T1>());
			break;

		case Expression_Operator_NOT_EQUAL:
			std::transform(a, b, c, d, std::not_equal_to<T1>());
			break;


		default:
			assert(false && "incorrect bin operation");

	}

}

template<typename T, typename R>
inline void ExpressionExecuter::evalUnOp(const size_t blockSize, const std::vector<void*> &sourceColumns,
		void *column, const Expression_Operator &expressionOperator) {

	// alocate (if needed) child and evaluate in its block
	this->allocateIfNotAllocatedChildColumn<T>(0);
	this->childrenExecuters[0]->eval(blockSize, sourceColumns, this->childrenColumns[0]);
	
	T *a = (T*) this->childrenColumns[0], *b = a + blockSize;
	R *c = (R*) column;

	switch (expressionOperator) {

		case Expression_Operator_LOG:
			std::transform(a,b,c, log);
			break;

		default:
			assert(false && "incorrect unary operator");
	}
}

template<typename T>
inline void ExpressionExecuter::evalUnOpInPlace(const size_t blockSize, const std::vector<void*> &sourceColumns,
		void *column, const Expression_Operator &expressionOperator) {

	// evaluate child in result block
	this->childrenExecuters[0]->eval(blockSize, sourceColumns, column);

	T *a = (T*) column, *b = a + blockSize;

	switch (expressionOperator) {
		case Expression_Operator_NEGATE:
			std::transform(a, b, a, std::negate<T>());
			break;

		case Expression_Operator_LOG:
			std::transform(a, b, a, log);
			break;

		case Expression_Operator_NOT:
			std::transform(a, b, a, std::logical_not<T>());
			break;

		default:
			assert(false && "incorrect unary operator");

	}
}

// generic function evaluating binary operator
// expression, when evaluating it evaluates the child with index indexInPlace (of type IN_PLACE_T)
// in result block and the second child (of type SEC_T) in its own block and then performs
// the operation in place 
// the type of SEC_T must be implicitly castable to IN_PLACE_T 
template<typename IN_PLACE_T, typename SEC_T>
inline void ExpressionExecuter::evalBinOpInPlace(const size_t blockSize, const std::vector<void*> &sourceColumns, 
		void* column, const Expression_Operator &expressionOperator, const int indexInPlace) {

	int secondIndex = 1 - indexInPlace;

	// evaluate in place child in result block
	this->childrenExecuters[indexInPlace]->eval(blockSize, sourceColumns, column);

	// allocate (if needed) second child and evaluate it in its block
	this->allocateIfNotAllocatedChildColumn<SEC_T>(secondIndex);
	this->childrenExecuters[secondIndex]->eval(blockSize, sourceColumns, this->childrenColumns[secondIndex]);

	IN_PLACE_T *a = (IN_PLACE_T*) column, *b = a + blockSize;
	SEC_T *c = (SEC_T*) this->childrenColumns[secondIndex], *d = c + blockSize;

	switch (expressionOperator) {

		case Expression_Operator_ADD:
			std::transform(a, b, c, a, std::plus<IN_PLACE_T>());
			break;

		case Expression_Operator_SUBTRACT:
			if (indexInPlace == 0)				// substracting is not commutative
				std::transform(a, b, c, a, std::minus<IN_PLACE_T>());
			else
				std::transform(c, d, a, a, std::minus<IN_PLACE_T>());
			break;

		case Expression_Operator_MULTIPLY:
			std::transform(a, b, c, a, std::multiplies<IN_PLACE_T>());
			break;

		case Expression_Operator_FLOATING_DIVIDE:
			if (indexInPlace == 0)				// division is not commutative
				std::transform(a, b, c, a, std::divides<IN_PLACE_T>());
			else
				std::transform(c, d, a, a, std::divides<IN_PLACE_T>());
			break;

		case Expression_Operator_LOWER:
			if (indexInPlace == 0)				// lower comparison is not commutative
				std::transform(a, b, c, a,std::less<IN_PLACE_T>());
			else
				std::transform(c, d, a, a, std::less<IN_PLACE_T>());
			break;

		case Expression_Operator_GREATER:
			if (indexInPlace == 0)
				std::transform(a, b, c, a, std::greater<IN_PLACE_T>());
			else
				std::transform(c, d, a, a, std::greater<IN_PLACE_T>());
			break;

		case Expression_Operator_EQUAL:
				std::transform(a, b, c, a, std::equal_to<IN_PLACE_T>());
				break;

		case Expression_Operator_NOT_EQUAL:
				std::transform(a, b, c, a, std::not_equal_to<IN_PLACE_T>());
				break;

		case Expression_Operator_AND:
				std::transform(a, b, c, a, std::logical_and<IN_PLACE_T>());
				break;

		case Expression_Operator_OR:
				std::transform(a, b, c, a, std::logical_or<IN_PLACE_T>());
				break;

		default:
			assert(false && "given operator is not one of plus, minus or multiplies");
		}
}

void ExpressionExecuter::eval(const size_t blockSize, const std::vector<void*> &sourceColumns, void* /*const*/ column) {
	Expression_Operator expressionOperator = this->expr.operator_();
	switch (expressionOperator) {

		case Expression_Operator_CONSTANT:
			if (this->expr.has_constant_int32()) 
				std::fill_n( (int*) column, blockSize, this->expr.constant_int32());
			else if (this->expr.has_constant_double()) 
				std::fill_n( (double*) column, blockSize, this->expr.constant_double());
			
			else if (this->expr.has_constant_bool()) 
				std::fill_n( (bool*) column, blockSize, this->expr.constant_bool());
			else
				assert(false && "wrong expression constant");
			break;

		case Expression_Operator_COLUMN:
			{
				int columnId = this->expr.column_id();

				switch (this->resultType) {
					case INT:
						std::copy( (int*) sourceColumns[columnId], (int*) sourceColumns[columnId] + blockSize, (int*) column );
						break;
					case DOUBLE:
						std::copy( (double*) sourceColumns[columnId], (double*) sourceColumns[columnId] + blockSize, (double*) column);
						break;
					case BOOL:
						std::copy( (bool*) sourceColumns[columnId], (bool*) sourceColumns[columnId] + blockSize, (bool*) column);
						break;
				}
			}
			break;

		case Expression_Operator_IF:
		
			assert(this->childrenTypes[1] == this->childrenTypes[2] && "types in then and else should be the same");

			if (this->childrenTypes[1] == INT) 
				this->evalIf<int>(blockSize, sourceColumns, column);
			else if (childrenTypes[1] == DOUBLE) 
				this->evalIf<double>(blockSize, sourceColumns, column);
			else 
				this->evalIf<bool>(blockSize, sourceColumns, column);

			break;

		case Expression_Operator_ADD:
		case Expression_Operator_SUBTRACT:
		case Expression_Operator_MULTIPLY:

			if (childrenTypes[0] == INT && childrenTypes[1] == INT) 
				this->evalBinOpInPlace<int,int>(blockSize, sourceColumns, column, expressionOperator, 0);	
			else if (childrenTypes[0] == DOUBLE && childrenTypes[1] == DOUBLE) 
				this->evalBinOpInPlace<double,double>(blockSize, sourceColumns, column, expressionOperator, 0);
			else if (childrenTypes[0] == DOUBLE && childrenTypes[1] == INT) 
				this->evalBinOpInPlace<double,int>(blockSize, sourceColumns, column, expressionOperator, 0);
			else if (childrenTypes[0] == INT && childrenTypes[1] == DOUBLE)
				this->evalBinOpInPlace<double,int>(blockSize, sourceColumns, column, expressionOperator, 1);
			else 
				assert(false && "incorrect types for plus/minus/multiplies expression");
			break;

		case Expression_Operator_FLOATING_DIVIDE:
			if (childrenTypes[0] == INT && childrenTypes[1] == INT) 
				this->evalBinOp<int,int,double>(blockSize, sourceColumns, column, expressionOperator);
			else if (childrenTypes[0] == DOUBLE && childrenTypes[1] == DOUBLE) 
				this->evalBinOpInPlace<double,double>(blockSize, sourceColumns, column, expressionOperator, 0);
			else if (childrenTypes[0] == DOUBLE && childrenTypes[1] == INT) 
				this->evalBinOpInPlace<double,int>(blockSize, sourceColumns, column, expressionOperator, 0);
			else if (childrenTypes[0] == INT && childrenTypes[1] == DOUBLE) 
				this->evalBinOpInPlace<double,int>(blockSize, sourceColumns, column, expressionOperator, 1); 
			else
				assert(false && "incorrect types for divide expression");
			break;

		case Expression_Operator_NEGATE:
			if (childrenTypes[0] == INT)
				this->evalUnOpInPlace<int>(blockSize, sourceColumns, column, expressionOperator);
			else if (childrenTypes[0] == DOUBLE) 
				this->evalUnOpInPlace<double>(blockSize, sourceColumns, column, expressionOperator);
			else
				assert("incorrect type in negate expression");

			break;

		case Expression_Operator_LOG: 
	
			switch (childrenTypes[0]) {

				case INT:
					this->evalUnOp<int,double>(blockSize, sourceColumns, column, expressionOperator);
					break;

				case DOUBLE:
					this->evalUnOpInPlace<double>(blockSize, sourceColumns, column, expressionOperator);
					break;

				default:
					assert(false && "incorrect type in log expression");
			}
			break;

		case Expression_Operator_NOT:
			assert(childrenTypes[0] == BOOL);
			this->evalUnOpInPlace<bool>(blockSize, sourceColumns, column, expressionOperator);
			break;

		case Expression_Operator_OR:
		case Expression_Operator_AND:
			assert(childrenTypes[0] == BOOL && childrenTypes[1] == BOOL);
			this->evalBinOpInPlace<bool,bool>(blockSize, sourceColumns, column, expressionOperator, 0);
			break;

		case Expression_Operator_LOWER:
		case Expression_Operator_GREATER:
		case Expression_Operator_EQUAL:
		case Expression_Operator_NOT_EQUAL:

			assert(childrenTypes[0] == childrenTypes[1] && "comparison operators should have arguments of the same type");

			switch (childrenTypes[0]) {
				case INT:
					this->evalBinOp<int,int,bool>(blockSize, sourceColumns, column, expressionOperator);
					break;

				case DOUBLE:
					this->evalBinOp<double, double, bool>(blockSize, sourceColumns, column, expressionOperator);
					break;

				case BOOL:
					this->evalBinOpInPlace<bool, bool>(blockSize, sourceColumns, column, expressionOperator, 0);
					break;
			}
			break;


		default:
			assert(false && "incorrect operator in expression");
	}

}

