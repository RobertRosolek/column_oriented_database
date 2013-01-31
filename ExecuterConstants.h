#ifndef PROJECT1_EXECUTERCONSTANTS_H
#define PROJECT1_EXECUTERCONSTANTS_H

// we will treat a block of this length as atomic 
// unit that we will perform operations on
const size_t BLOCK_LEN = 3500;

// at least (FILTER_PROPORTION_NUMERATOR / FILTER_PROPORTION_DENOMINATOR) fraction of requested block size
// must be supplied by filter executer
const int FILTER_PROPORTION_NUMERATOR = 1, FILTER_PROPORTION_DENOMINATOR = 2;

#endif // PROJECT1_EXECUTERCONSTANTS_H
