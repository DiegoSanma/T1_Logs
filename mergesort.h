#ifndef MergeSort_H
#define MergeSort_H

class MergeSort {
    private:
        const char* filename;
        int alfa;
        int B;
        MergeSort * hijos;
    
    public:
        MergeSort(const char* filename, int alfa);
    
       const char* getFileName() const;
    
        int getAlfa() const;
    
        int MergeSortN(size_t largo,int inicio,int M) const;
    };
    
    #endif