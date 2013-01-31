#include <vector>

#include "operations.pb.h"
#include "Types.h"
#include "OperationTypeVisitor.h"
#include "ExpressionTypeVisitor.h"

/**************************/
/***** Static methods *****/
/**************************/

void OperationTypeVisitor::getType(const Operation &query, std::vector<Type> &types) {
	if (query.has_scan()) 
		OperationTypeVisitor::getType(query.scan(), types);
	else if (query.has_compute())
		OperationTypeVisitor::getType(query.compute(), types);
	else if (query.has_filter())
		OperationTypeVisitor::getType(query.filter(), types);
	else if (query.has_group_by())
		OperationTypeVisitor::getType(query.group_by(), types);
	else
		assert(false && "incorrect type of operation");
}

void OperationTypeVisitor::getType(const ScanOperation &query, std::vector<Type> &types) {
	int s = query.type_size();
	types.resize(s);
	for (int i = 0; i < s; ++i)
		types[i] = (Type) query.type(i); // Type enum is defined in a way that this casting is good for sure
}

void OperationTypeVisitor::getType(const ComputeOperation &query, std::vector<Type> &types) {
	std::vector<Type> sourceTypes;

	// get the source types
	OperationTypeVisitor::getType(query.source(), sourceTypes);

	int s = query.expressions_size();
	types.resize(s);

	// get result types
	for (int i = 0; i < s; ++i)
		types[i] = ExpressionTypeVisitor::getType(query.expressions(i), sourceTypes);
}

void OperationTypeVisitor::getType(const FilterOperation &query, std::vector<Type> &types) {
	// type of filter operation does depend only on source and not on filter expression
	OperationTypeVisitor::getType(query.source(), types);
}

void OperationTypeVisitor::getGroupedByTypes(const GroupByOperation &query, const std::vector<Type> &sourceTypes, 
		std::vector<Type> &types) {
	
	int groupBySize = query.group_by_column_size();

	types.resize(groupBySize);

	for (int i = 0; i < groupBySize; ++i)
		types[i] = sourceTypes[query.group_by_column(i)];
}

void OperationTypeVisitor::getAggregationsTypes(const GroupByOperation &query, const std::vector<Type> &sourceTypes, 
		std::vector<Type> &types) {

	int aggregationsSize = query.aggregations_size();

	types.resize(aggregationsSize);

	for (int i = 0; i < aggregationsSize; ++i)
		if (query.aggregations(i).type() == Aggregation_Type_SUM) // for sum the type of aggregation is the same as the aggregated column
			types[i] = sourceTypes[query.aggregations(i).aggregated_column()];
		else if (query.aggregations(i).type() == Aggregation_Type_COUNT) // for count rhe type is always int
			types[i] = INT;
		else
			assert(false && "incorrect aggregation type");
}

void OperationTypeVisitor::getType(const GroupByOperation &query, std::vector<Type> &types) {
	std::vector<Type> sourceTypes, groupedByTypes, aggregationsTypes;

	// get the source types
	OperationTypeVisitor::getType(query.source(), sourceTypes);

	// get grouped by types
	OperationTypeVisitor::getGroupedByTypes(query, sourceTypes, groupedByTypes);

	// get aggretation types
	OperationTypeVisitor::getAggregationsTypes(query, sourceTypes, aggregationsTypes);

	// first goes grouped by types and then aggregation types
	types.insert(types.end(), groupedByTypes.begin(), groupedByTypes.end());
	types.insert(types.end(), aggregationsTypes.begin(), aggregationsTypes.end());
}
