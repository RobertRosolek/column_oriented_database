#include <vector>
#include <cassert>

#include "operations.pb.h"
#include "Types.h"
#include "ExpressionTypeVisitor.h"

/**************************/
/***** Static methods *****/
/**************************/

Type ExpressionTypeVisitor::getType(const Expression &query, const std::vector<Type> &sourceType) {

	// children types
	Type t1, t2;

	if (query.has_constant_int32())
		return INT;
	else if (query.has_constant_double())
		return DOUBLE;
	else if (query.has_constant_bool())
		return BOOL;
	else if (query.has_column_id())
		return sourceType[query.column_id()];
	else 
		switch (query.operator_()) {

			// type of then expression  
			case Expression_Operator_IF:
				return ExpressionTypeVisitor::getType(query.children(1), sourceType);

			// double if either of the children is double, int otherwise
			case Expression_Operator_ADD:
			case Expression_Operator_SUBTRACT:
			case Expression_Operator_MULTIPLY:
				t1 = ExpressionTypeVisitor::getType(query.children(0), sourceType);
				t2 = ExpressionTypeVisitor::getType(query.children(1), sourceType);
				if (t1 == DOUBLE || t2 == DOUBLE)
					return DOUBLE;
				return INT;

			// always double
			case Expression_Operator_LOG:
			case Expression_Operator_FLOATING_DIVIDE:
				return DOUBLE;

			// type of a child
			case Expression_Operator_NEGATE:
				return ExpressionTypeVisitor::getType(query.children(0), sourceType);	

			// always bool
			case Expression_Operator_LOWER:
			case Expression_Operator_GREATER:
			case Expression_Operator_EQUAL:
			case Expression_Operator_NOT_EQUAL:
			case Expression_Operator_NOT:
			case Expression_Operator_OR:
			case Expression_Operator_AND:
				return BOOL;

			default: 							
				assert(false && "unrecognised operator");
				return INT;
		}	
}



