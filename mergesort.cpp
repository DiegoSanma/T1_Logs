#include "mergesort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

MergeSort::MergeSort(const char* filename, int alfa){
    this->filename = filename;
    this->alfa = alfa;
}

const char * MergeSort::getFileName() const{
    return this->filename;
}

int MergeSort::getAlfa() const{
    return this->alfa;
}

int MergeSort::MergeSortN(size_t largo,int inicio,int M) const{
    std::ifstream archivo(filename, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo binario." << std::endl;
        return 1;
    }

    if(largo>M){
        for(int i = 0; i<this->alfa;++i){
            int start = i*this->alfa +1;
            size_t largo_nuevo = largo/this->alfa;
            //El de este sea raiz, y que los demás MergeSort sean sus hojas
            MergeSort hijo(filename,this->alfa);
            this->hijos[i] = hijo;
            hijo.MergeSortN(largo_nuevo,start,M);
            archivo.close();
        }
        for(int i = 0;i<this->alfa;++i){
            //Unir hijos
            return 0;
        }
    }
    else{
        //Lo ordeno aquí en "memoria princiapl"
        int pos_final = inicio + largo;
        int cantidad = pos_final - inicio + 1;             // cuántos enteros quieres leer
        std::vector<uint64_t> buffer(cantidad);                   // buffer para guardarlos

        //Leo la cosa
        archivo.seekg(inicio * sizeof(int), std::ios::beg);   
        archivo.read(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        sort(buffer.begin(),buffer.end());
        archivo.close();
        //Dsp de ordenar, lo reescribo en el archivo
        std::ofstream archivoFuera(filename, std::ios::binary);
        if (!archivoFuera.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
            std::exit(1);
        }
        archivoFuera.seekp(inicio * sizeof(int), std::ios::beg);
        archivoFuera.write(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        archivoFuera.close();
    }
    return 0;

}
