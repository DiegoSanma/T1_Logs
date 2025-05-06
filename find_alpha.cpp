#include "mergesort.h"
#include "crear_array.h"
#include "find_alpha.h"
#include <limits>
#include <iostream>

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
int findOptimalArity(int b, const char* filename, int M, int X,size_t B) {
    // Initialize binary search bounds
    int left = 2, right = b;
    //int optimalAlpha = left;
    int minIOs = std::numeric_limits<int>::max();
    int i=0;
    MergeSort best;

    // Binary search for the optimal arity
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int c = std::max(1, (right - left) / 4); // Adjust c relative to the range
        int midMinusC = std::max(left, mid - c);
        int midPlusC = std::min(right, mid + c);
        //Defino los IOs que voy a calcular
        int IOs_mergeMinusC;
        int IOs_mergePlusC;
        //Defino los Mergesort que voy a hacer
        MergeSort* mergesortMinusC;
        MergeSort* mergesortPlusC;
        // Create the array using the provided filename and parameters
        std:: cout << "Creando el arreglo ..." << std::endl;
        CrearArray creador(filename, M, X);
        creador.crearArrayN();
        size_t largo_archivo = static_cast<size_t>(X) * M * 1024 * 1024;
            
        //Reviso cual
        if(i){
            if(midMinusC==best.getAlfa()){
                IOs_mergeMinusC = minIOs;
                MergeSort mergesortMinusC = best;
                std::cout << "Haciendo Mergesort con: " << midPlusC << std::endl;
                MergeSort mergesortPlusC(filename, midPlusC, largo_archivo, 0,B);
                int IOs_mergePlusC = mergesortPlusC.MergeSortN(M,B);
                std::cout << "El número de IOs para este MergeSort es: " << IOs_mergePlusC << std::endl;
            }
            else{
                IOs_mergePlusC = minIOs;
                MergeSort mergesortPlusC = best;
                std::cout << "Haciendo Mergesort con: " << midMinusC << " para compararlo con " << best.getAlfa() << std::endl;
                MergeSort mergesortMinusC(filename, midMinusC, largo_archivo, 0,B);
                int IOs_mergeMinusC = mergesortMinusC.MergeSortN(M,B);
                std::cout << "El número de IOs para este MergeSort es: " << IOs_mergeMinusC << std::endl;
            }
        } else {
            // Uncomment the following lines to duplicate the .bin file for consistency
            // std::string duplicateFilename = std::string(filename) + "_copy";
            // std::ifstream src(filename, std::ios::binary);
            // std::ofstream dst(duplicateFilename, std::ios::binary);
            // dst << src.rdbuf();
            // src.close();
            // dst.close();
            
            // Perform mergesort with midMinusC
            std::cout << "Haciendo Mergesort con: " << midMinusC << std::endl;
            MergeSort mergesortMinusC(filename, midMinusC, largo_archivo, 0,B);
            int IOs_mergeMinusC = mergesortMinusC.MergeSortN(M,B);
            std::cout << "El número de IOs para este MergeSort es: " << IOs_mergeMinusC << std::endl;
            
            // Perform mergesort with midPlusC
            // Uncomment the following line to use the duplicated file for consistency
            // MergeSort mergesortPlusC(duplicateFilename.c_str(), midPlusC, largo_archivo, 0);
            std::cout << "Haciendo Mergesort con: " << midPlusC << std::endl;
            MergeSort mergesortPlusC(filename, midPlusC, largo_archivo, 0,B);
            int IOs_mergePlusC = mergesortPlusC.MergeSortN(M,B);
            std::cout << "El número de IOs para este MergeSort es: " << IOs_mergePlusC << std::endl;
        }
        
        // Compare IOs and decide the direction of binary search
        // Logic can be adjusted
        if (IOs_mergeMinusC < IOs_mergePlusC) {
            if (IOs_mergeMinusC < minIOs) {
                minIOs = IOs_mergeMinusC;
                //optimalAlpha = midMinusC;
                best = *mergesortMinusC;
                i++;
            }
            right = mid - 1; // Move to the lower half
        } else {
            if (IOs_mergePlusC < minIOs) {
                minIOs = IOs_mergePlusC;
                //optimalAlpha = midPlusC;
                best = *mergesortPlusC;
                i++;
            }
            left = mid + 1; // Move to the upper half
        }
    }

    return best.getAlfa();
}
