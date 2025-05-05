#include "crear_array.h"
#include "mergesort.h"
#include "find_alpha.h"
#include <iostream>


int main(){
    int M = 50;
    size_t B = 4096; //4096kb
    //Primero, inicializo la clase que crea el arreglo
    //CrearArray creador("arreglo.bin",M,60);
    std::cout << "Hola desde Docker!" << std::endl;
    //Ahora, debo sacar la aridad usando MergeSort
    int alfa = findOptimalArity(B,"arreglos_aridad.bin",M,60,B);
    std::cout << "alfa: " << alfa << std::endl;
    //Ahora usando ese alfa, debo realizar MergeSort y QuickSort
    //Lo comento pa q cuando lo corran solo veamos primero la aridad
    //for(int i=4;i<=60;i+=4){
    //    creador.setX(i);
    //    for(int j=0;j<5;++j){
    //        size_t largo_archivo = M*i*1024*1024;
    //        //Primero creamos la secuencia de i * M numeros de 64 bits
    //        creador.crearArrayN();
    //        //Dsp tenemos que hacerle mergesort y quicksort, y guardar los resultados
    //        MergeSort mergesort("arreglo.bin",alfa,largo_archivo,0,B);
    //        int IOs_merge = mergesort.MergeSortN(M,B);
    //        //Finalmente, borro lo que había en el archivo.bin y sigo con la próxima secuencia
    //    
    //    }
    //}
    //std::cout << "Hola desde Docker2!" << std::endl;

    return 0;
}