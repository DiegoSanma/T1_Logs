#include "crear_array.h"
#include "mergesort.h"
#include "find_alpha.h"
#include "quicksort.h"
#include <iostream>
#include <csignal>
#include <cstdio>

static const char* FILE_ALPHA   = "arreglos_aridad.bin";

static void delete_temp_files()
{
    std::remove(FILE_ALPHA);
}

int findMaxArity(int M, int B) {
    constexpr int MIN_ARITY = 2;                 // assignment lower bound
    constexpr size_t WORD     = sizeof(uint64_t);  // element size in bytes

    /* 1· Memory‑capacity bound  ⌊ M/B ⌋ – 1 */
    int maxByMemory = (M * 1024 / B) - 1;          // M in MB, B in KB
    if (maxByMemory < MIN_ARITY)  maxByMemory = MIN_ARITY;

    /* 2· Pivot‑block bound      ⌊ blockBytes / elementSize ⌋ */
    int maxByBlock  = (B * 1024) / WORD;           // elements in one disk block
    if (maxByBlock  < MIN_ARITY)  maxByBlock  = MIN_ARITY;

    /* 3· Final upper limit = stricter of the two */
    return std::min(maxByMemory, maxByBlock);
}

int main(){
    int    M = 50;
    size_t B = 4096; //4096kb
    // create an array of ints from 4 to 60 by 4
    // int *arreglo = new int[15];
    // for (int i = 0; i < 15; ++i) {
    // for (int i = 0; i < 15; ++i) {
    //     arreglo[i] = 4 + i * 4;
    // }
    int Xtest = 4;

    //Primero, inicializo la clase que crea el arreglo
    const char * filename = "arreglo.bin";
    CrearArray creador(filename,M,4);

    std::cout << "Hola desde Docker!" << std::endl;
    //Ahora, debo sacar la aridad usando MergeSort

    int maxArity = findMaxArity(M,B); // 2 <= maxArity
    std::cout << "maxArity: " << maxArity << std::endl;

    /*---------------------------------------------------------------
     *  Now call the optimiser inside the safe interval [2, maxArity]
     *-------------------------------------------------------------*/
    std::signal(SIGINT,  [](int){ delete_temp_files(); std::exit(130); });
    std::signal(SIGTERM, [](int){ delete_temp_files(); std::exit(143); });
    int alfa = findOptimalArity(maxArity,
                                "arreglos_aridad.bin",
                                M,
                                Xtest,     // X (array size factor)
                                B);        // block size KB
    std::cout << "alfa: " << alfa << std::endl;
    creador.crearArrayN();
    //int alfa = findOptimalArity(B,"arreglos_aridad.bin",M,60,B);
    QuickSort quicksort(filename,alfa,M*Xtest*1024*1024,0,B);
    std::cout << "Empiezo quicksort..." << std::endl;
    int IOs = quicksort.QuickSortN(M,B);
    std::cout << "Usaron esta cantidad de IOs:" << IOs << std::endl;

    //std::cout << "alfa: " << alfa << std::endl;
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

    delete_temp_files();

    return 0;
}