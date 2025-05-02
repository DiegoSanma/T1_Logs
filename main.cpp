#include "crear_array.h"
#include "mergesort.h"
#include <iostream>


int main(){
    int M = 50;
    //Primero, inicializo la clase que crea el arreglo
    CrearArray creador("arreglo.bin",M,4);
    std::cout << "Hola desde Docker!" << std::endl;
    for(int i=4;i<=60;i+=4){
        creador.setX(i);
        for(int j=0;j<5;++j){
            size_t largo_archivo = M*i*1024*1024;
            //Primero creamos la secuencia de i * M numeros de 64 bits
            creador.crearArrayN();
            //Dsp tenemos que hacerle mergesort y quicksort, y guardar los resultados
            MergeSort mergesort("arreglo.bin",2,largo_archivo,0);
            int IOs_merge = mergesort.MergeSortN(M);
            //Finalmente, borro lo que había en el archivo.bin y sigo con la próxima secuencia
        
        }

    }
    std::cout << "Hola desde Docker2!" << std::endl;

    return 0;
}