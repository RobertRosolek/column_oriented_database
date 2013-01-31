using namespace std;

enum Type {
	BOOL = 0,
	INT = 1
};

template<Type t> 
struct TypeTraits {

};

template<>
struct TypeTraits<BOOL> {
	typedef bool T;
};

template<> 
struct TypeTraits<INT> {
	typedef int T;
};

enum Operation {
	ADD = 0
};

template<Operation Op>
struct OperationTraits {

};


template<>
struct OperationTraits<ADD> {
	template<typename T1, typename T2, typename T3>
	T3 oper (T1 a, T2 b) {
		return a + b;
	}	
};

template <Type inT1, Type inT2, Type outT, Operation op>
void add(typename TypeTraits<inT1>::T *a, typename TypeTraits<inT2>::T *b, typename TypeTraits<outT>::T *c, int block_len) {
	static OperationTraits<op> operation;
	for (int i = 0; i < block_len; ++i)
		c[i] =  operation. template oper<int,int,int>(a[i], b[i]);
}	

const int BLOCK_LEN = 100;

int main() {

	

	int *a = new int[BLOCK_LEN], *b = new int[BLOCK_LEN], *c = new int[BLOCK_LEN];

	add<INT,INT,INT,ADD>(a,b,c, BLOCK_LEN);


	return 0;
}
