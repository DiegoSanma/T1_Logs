#ifndef MergeSort_H
#define MergeSort_H


class MergeSort {
    private:
        const char* filename;
        int alfa;
        size_t largo;
        int inicio;
        MergeSort * hijos;
    
    public:
        MergeSort(const char* filename, int alfa, size_t largo,int inicio);

        const char* getFileName() const;

        int getAlfa() const;

        size_t getLargo() const;

        int getInicio() const;
    
        int MergeSortN(int M) const;

        int unionHijos(int M, std::ifstream& archivo) const;
    };
    
    #endif