#include <iostream>
#include <fstream>
#include "crear_array.h"
#include <string>
#include <random>
#include <vector>
#include <cstdint>
#include <cstdio> 


CrearArray::CrearArray(const char* filename, int M,int X){
        this->filename = filename;
        this->M = M;
        this->X = X;

    }

const char * CrearArray::getFileName() const{
        return this->filename;
   }

int CrearArray::getM() const{
        return this->M;
   }

int CrearArray::getX() const{
    return this->X;
}

void CrearArray::setX(int x){
    X = x;
}

int CrearArray::crearArrayN() const{
        //Primero, abro el archivo para escribirlo en disco
        std::ofstream archivo(filename, std::ios::binary);
        if (!archivo.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
            std::exit(1);
        }
        //Creo el arreglo y el generador de números aleatorios
        const size_t bytes_ram = M *1024 *1024;
        //Para todas las iteraciones X*M 
        int cant_numeros = 0;
        const size_t bits64 = (bytes_ram*X)/sizeof(uint64_t);
        std::mt19937_64 rng(42); // Fixed seed
        std::uniform_int_distribution<uint64_t> dist;
        for(size_t i=0;i<bits64;++i){
            uint64_t numero = dist(rng);
            archivo.write(reinterpret_cast<const char*>(&numero),sizeof(uint64_t));
        }
        cant_numeros += 1;
        std::cout << "El número es: " << cant_numeros << std::endl;
    archivo.close();
    return 0;    
}