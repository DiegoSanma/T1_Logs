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
    int left = 100, right = b;
    std::vector<int> IOs_por_aridad(b+1,0);
    //int optimalAlpha = left;
    int minIOs = std::numeric_limits<int>::max();
    int optimalAlpha = left;

    std::unordered_map<int, int> arityIOMap;

    std::cout << "Creando el arreglo ..." << std::endl;
    CrearArray creador(filename, M, X);
    creador.crearArrayN();

    size_t fileSize = static_cast<size_t>(X) * M * 1024 * 1024;

    while (left < right) {
        int midMinusC = left;
        int midPlusC = right;
        int mid = (right+left)/2;

        std::cout << "Evaluando aridad: " << midMinusC << " y " << midPlusC << std::endl;

        if (!arityIOMap.count(midMinusC))
            arityIOMap[midMinusC] = runMergeSort(filename, midMinusC, fileSize, M, B);
        int IOsMinusC = arityIOMap[midMinusC];
        std::cout << "El número de IOs para el MergeSort es: " << IOsMinusC << std::endl;
        
        if (!arityIOMap.count(midPlusC))
            arityIOMap[midPlusC] = runMergeSort(filename, midPlusC, fileSize, M, B);
        int IOsPlusC = arityIOMap[midPlusC];
        std::cout << "El número de IOs para el MergeSort es: " << IOsPlusC << std::endl;

        arityIOMap.emplace(midPlusC, IOsPlusC);

        if (IOsMinusC < IOsPlusC) {
            if (IOsMinusC < minIOs) {
                minIOs = IOsMinusC;
                optimalAlpha = midMinusC;
            }
            right = mid;
        } else {
            if (IOsPlusC < minIOs) {
                minIOs = IOsPlusC;
                optimalAlpha = midPlusC;
            }
            left = mid;
        }
        std::cout << "El mejor es " << optimalAlpha << std::endl;
    }

    std::cout << "Optimal arity: " << optimalAlpha << " with IOs: " << minIOs << std::endl;
    return optimalAlpha;
}