#ifndef Quicksort_H
#define Quicksort_H

#include <cstddef>   // for size_t
#include <vector>    // for std::vector
#include <fstream>   // for std::ifstream, std::ofstream
#include <cstdint>   // if you're using uint64_t

class QuickSort {
    private:
        const char* filename;
        int alfa;
        size_t largo;
        int inicio;
        std::vector<QuickSort> hijos;
        size_t B;
        mutable size_t pos_qs_;
    
    public:
        QuickSort(const char* filename, int alfa, size_t largo,int inicio,size_t B);

        QuickSort() = default;

        const char* getFileName() const;

        int getAlfa() const;

        size_t getLargo() const;

        int getInicio() const;
    
        int QuickSortN(int M, size_t B);

        int qsHijos(int M, size_t B, std::ifstream& archivo) const;
};

#endif