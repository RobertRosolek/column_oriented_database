#ifndef PROJECT1_OPERATIONS_H
#define PROJECT1_TYPES_H

#include <iostream>
#include <algorithm>
#include <functional>

#include "operations.pb.h"

template<Expression_Operator op> struct BinOp {};

template<> struct BinOp<Expression_Operator_ADD> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a + b; }
};

template<> struct BinOp<Expression_Operator_SUBTRACT> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a - b; }
};

template<> struct BinOp<Expression_Operator_MULTIPLY> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a * b; } 
};

template<> struct BinOp<Expression_Operator_FLOATING_DIVIDE> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a / b; }
};

template<> struct BinOp<Expression_Operator_LOWER> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return std::less<T1>(a,b); }
};

template<> struct BinOp<Expression_Operator_GREATER> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return std::greater<T1>(a,b); }
};

template<> struct BinOp<Expression_Operator_EQUAL> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a == b; }	
};

template<> struct BinOp<Expression_Operator_NOT_EQUAL> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a != b; }
};

template<> struct BinOp<Expression_Operator_OR> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) { return a || b; }
};

template<> struct BinOp<Expression_Operator_AND> {
	template<typename T1, typename T2, typename R> static R op(T1 a, T2 b) {return a && b; }
};




#endif // PROJECT1_OPERATIONS_H
