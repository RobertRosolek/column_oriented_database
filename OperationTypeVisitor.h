#ifndef PROJECT1_OPERATIONTYPEVISITOR_H
#define PROJECT1_OPERATIONTYPEVISITOR_H

#include <vector>

#include "operations.pb.h"
#include "Types.h"

// visitor-like class that traverses the tree of operation description
//  and gets the type of the result (which is a product (represented by vector)
//  of atomic types) of given operation
class OperationTypeVisitor {
	public:
		static void getGroupedByTypes(const GroupByOperation &query, const std::vector<Type> &sourceTypes, 
				std::vector<Type> &types);
		static void getAggregationsTypes(const GroupByOperation &query, const std::vector<Type> &sourceTypes,
				std::vector<Type> &types);

		static void getType(const Operation &query, std::vector<Type> &types);
		static void getType(const ScanOperation &query, std::vector<Type> &types);
		static void getType(const ComputeOperation &query, std::vector<Type> &types);
		static void getType(const FilterOperation &query, std::vector<Type> &types);
		static void getType(const GroupByOperation &query, std::vector<Type> &types);
};


#endif // PROJECT1_OPERATIONTYPEVISITOR_H
