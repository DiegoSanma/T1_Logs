#ifndef SortData_H
#define SortData_H

#include <cstddef>   // for size_t

struct SortData {
    size_t arraySize;
    double time;
    int IOs;
    int arity;
};

using MergeSortData = SortData;
using QuickSortData = SortData;

#endif // SortData_H
