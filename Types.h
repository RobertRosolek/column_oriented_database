#ifndef PROJECT1_TYPES_H
#define PROJECT1_TYPES_H

#include <iostream>
#include <algorithm>
#include <functional>

#include "operations.pb.h"
#include "ExecuterConstants.h"

/*****************/
/***** Types *****/
/*****************/

enum Type {
	INT = ScanOperation_Type_INT,
	DOUBLE = ScanOperation_Type_DOUBLE,
	BOOL = ScanOperation_Type_BOOL
};

inline int typeSize(Type t) {
	switch (t) {
		case INT:
			return sizeof(int);

		case DOUBLE:
			return sizeof(double);

		case BOOL:
			return sizeof(bool);

		default:
			assert(false && "incorrect type");
	}
}

struct BlockAllocator : public std::binary_function<int,Type,void*> {
	
	void* operator() (const int &blockSize,const Type &type) const {
		switch (type) {
			case INT:
				return malloc(blockSize * sizeof(int));
			case DOUBLE:
				return malloc(blockSize * sizeof(double));
			default:
				return malloc(blockSize * sizeof(bool));
		}
	}
};



/*inline void* allocBlock(int &blockSize, Type &type) {
	switch (type) {
		case INT:
			return malloc(blockSize * sizeof(int));

		case DOUBLE:
			return malloc(blockSize * sizeof(double));

		default:
			return malloc(blockSize * sizeof(bool));
	}
}*/

// allocate full block of given type
inline void* allocFullBlock(Type t) {
	switch (t) {
		case INT:
			return malloc(BLOCK_LEN * sizeof(int));

		case DOUBLE:
			return malloc(BLOCK_LEN * sizeof(double));

		case BOOL:
			return malloc(BLOCK_LEN * sizeof(bool));

		default:
			assert(false && "incorrect type");
			return NULL;
	}
}

// allocate a vector of full blocks of given types
inline void allocFullBlocs(std::vector<void*> &columns, const std::vector<Type> &columnTypes) {
	//if (columns.size() != columnTypes.size()) 
	//	std::cerr << columns.size() << " " << columnTypes.size() << std::endl;
	assert(columns.size() == columnTypes.size());
	std::transform(columnTypes.begin(), columnTypes.end(), columns.begin(), allocFullBlock);
}


inline void outputType(Type t) {
	switch (t) {
		case INT: 
			std::cerr << "Int ";
			break;

		case DOUBLE:
			std::cerr << "Double ";
			break;

		case BOOL:
			std::cerr << "Bool ";
			break;

		default:
			assert(false && "incorrect type");
	}
}

inline void outputTypes(const std::vector<Type> &types) {
	std::for_each(types.begin(), types.end(), outputType);
	std::cerr << std::endl;
}

/***********************/
/***** Type Traits *****/
/***********************/

template<Type t>
struct  TypeTraits {
	
};

template<>
struct TypeTraits<INT> {
	typedef int T;
};

template<>
struct TypeTraits<DOUBLE> {
	typedef double T;
};

template<> 
struct TypeTraits<BOOL> {
	typedef bool T;
};





#endif // PROJECT1_TYPES_H
