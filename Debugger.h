#ifndef PROJECT1_DEBUGGER_H
#define PROJECT1_DEBUGGER_H

#include <iostream>
#include <iomanip>

#ifdef DEBUG_ON
static const bool debugFlag = true;
#else
static const bool debugFlag = false;
#endif

template<typename T>
static void outputData(void *s, size_t offset) {
	std::cout << std::setprecision(10) << ( *( (T*) s + offset));
}

inline void debugOutputBlock(const size_t blockSize, const std::vector<void*> &columns, const std::vector<Type> &columnTypes) {
	if (!debugFlag) return;
	for (size_t row = 0; row < blockSize; ++row) {
		for (size_t i = 0, noOfColumns = columns.size(); i < noOfColumns; ++i) {
			switch (columnTypes[i]) {
				case INT:
					outputData<int>(columns[i], row);
					break;
				case DOUBLE:
					outputData<double>(columns[i], row);
					break;
				default:
					;
					outputData<bool>(columns[i], row);
			}
			std::cout << " ";
		}
		std::cout << std::endl;
	}
}

#endif // PROJECT1_DEBUGGER_H

