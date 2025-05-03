#ifndef MergeSort_H
#define MergeSort_H


class MergeSort {
    private:
        const char* filename;
        int alfa;
        size_t largo;
        int inicio;
        MergeSort * hijos;
        size_t B;
    
    public:
        MergeSort(const char* filename, int alfa, size_t largo,int inicio,size_t B);

        const char* getFileName() const;

        int getAlfa() const;

        size_t getLargo() const;

        int getInicio() const;
    
        int MergeSortN(int M, size_t B) const;

        int lectura_bloques(size_t B,int cantidad_lectura,std::vector<uint64_t> &buffer,std::ifstream &archivo,int pos=0) const;

        int escritura_bloques(size_t B,int cantidad_lectura,std::vector<uint64_t> &buffer,std::ofstream &archivo) const;

        int unionHijos(int M, size_t B, std::ifstream& archivo) const;
    };
    
    #endif