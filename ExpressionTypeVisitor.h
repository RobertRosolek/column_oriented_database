#ifndef PROJECT1_EXPRESSIONTYPEVISITOR_H
#define PROJECT1_EXPRESSIONTYPEVISITOR_H

#include <vector>

#include "operations.pb.h"
#include "Types.h"

//  visitor-like class that traverses the tree of expression description
//  and gets the type of the result of given expression
class ExpressionTypeVisitor {
	public:
		static Type getType(const Expression &query, const std::vector<Type> &sourceTypes);
};


#endif // PROJECT1_EXPRESSIONTYPEVISITOR_H
