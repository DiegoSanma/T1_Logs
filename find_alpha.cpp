#include "mergesort.h"
#include "crear_array.h"
#include "find_alpha.h"
#include <limits>
#include <iostream>
#include <vector>
#include <unordered_map>

int runMergeSort(const std::string& filename, int arity, size_t fileSize, int M, size_t B) {
    std::cout << "Haciendo Mergesort con: " << arity << std::endl;
    MergeSort sorter(filename.c_str(), arity, fileSize, 0, B);
    return sorter.MergeSortN(M, B);
}

/**
 * Function to find the best arity (alpha) for mergesort using binary search
 * between [2, b], optimizing for minimum IOs.
 *
 * This function is designed to minimize the number of I/O operations during
 * mergesort by finding the optimal arity (alpha). Binary search is used to
 * efficiently explore the range of possible arities, as the relationship
 * between arity and I/O operations is typically non-linear but monotonic
 * within certain ranges.
 *
 * @param b The maximum arity to consider.
 * @param filename The name of the binary file used to create the array.
 * @param M The size of available RAM (in MB).
 * @param X The size of the array in units of (M * 1024 * 1024).
 * @param B Size of a block in memoery
 * @return The optimal arity (alpha) that minimizes I/O operations.
 */
int findOptimalArity(int b, const char* filename, int M, int X, size_t B) {
    int left = 2, right = b;
    int optimalAlpha = left;
    int minIOs = std::numeric_limits<int>::max();

    std::unordered_map<int, int> arityIOMap;

    std::cout << "Creando el arreglo ..." << std::endl;
    CrearArray creador(filename, M, X);
    creador.crearArrayN();

    size_t fileSize = static_cast<size_t>(X) * M * 1024 * 1024;

    while (left <= right) {
        int c = std::max(1, (right - left) / 4);
        int mid = left + (right - left) / 2;
        int midMinusC = std::max(left, mid - c);
        int midPlusC = std::min(right, mid + c);

        int IOsMinusC = (arityIOMap.count(midMinusC) > 0)
                        ? arityIOMap[midMinusC]
                        : runMergeSort(filename, midMinusC, fileSize, M, B);

        // Store it if not already there
        arityIOMap.emplace(midMinusC, IOsMinusC);

        int IOsPlusC = (arityIOMap.count(midPlusC) > 0)
                       ? arityIOMap[midPlusC]
                       : runMergeSort(filename, midPlusC, fileSize, M, B);

        arityIOMap.emplace(midPlusC, IOsPlusC);

        if (IOsMinusC < IOsPlusC) {
            if (IOsMinusC < minIOs) {
                minIOs = IOsMinusC;
                optimalAlpha = midMinusC;
            }
            right = mid - 1;
        } else {
            if (IOsPlusC < minIOs) {
                minIOs = IOsPlusC;
                optimalAlpha = midPlusC;
            }
            left = mid + 1;
        }
    }

    std::cout << "Optimal arity: " << optimalAlpha << " with IOs: " << minIOs << std::endl;
    return optimalAlpha;
}