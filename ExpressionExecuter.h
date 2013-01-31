#ifndef PROJECT1_EXPRESSIONEXECUTER_H
#define PROJECT1_EXPRESSIONEXECUTER_H

#include <vector>
#include <utility>

#include "server.h"
#include "operations.pb.h"
#include "Types.h"

class ExpressionExecuter {
	private:
		const Expression &expr;

		const std::vector<Type> &sourceColumnTypes;

		std::vector<Type> childrenTypes;
		std::vector<void*> childrenColumns;
		std::vector<bool> isAllocatedChildColumn;

		std::vector<ExpressionExecuter*> childrenExecuters;

		Type resultType;

		template<typename T>
		inline void evalIf(const size_t blockSize, const std::vector<void*> &sourceColumns, void* column);

		template<typename T, typename R>
		inline void evalUnOp(const size_t blockSize, const std::vector<void*> &sourceColumns, void *column,
				const Expression_Operator &expressionOperator);

		template<typename T>
		inline void evalUnOpInPlace(const size_t blockSize, const std::vector<void*> &sourceColumns, void *column,
				const Expression_Operator &expressionOperator);

		template<typename T1, typename T2, typename R>
		inline void evalBinOp(const size_t blockSize, const std::vector<void*> &sourceColumns, void *column,
				const Expression_Operator &expressionOperator);

		template<typename IN_PLACE_T, typename SEC_T>
		inline void evalBinOpInPlace(const size_t blockSize, const std::vector<void*> &sourceColumns, void *column, 
				const Expression_Operator &expressionOperator, const int indexInPlace);

		template<typename T>
		inline void allocateIfNotAllocatedChildColumn(const int index);

	public:	
		ExpressionExecuter(const Expression &expr, const std::vector<Type> &sourceColumnTypes);
		~ExpressionExecuter();

		void eval(const size_t blockSize, const std::vector<void*> &sourceColumns, void * /*const*/ column);
};


#endif // PROJECT1_EXPRESSIONEXECUTER_H
