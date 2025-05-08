#include "crear_array.h"
#include "mergesort.h"
#include "find_alpha.h"
#include "quicksort.h"
#include "data_structs.h"
#include <iostream>
#include <csignal>
#include <cstdio>
#include <chrono>

#include <chrono>
#include <ctime>
#include <iomanip>

#define M       50
#define B       4096 //4096kb
#define Xtest   60
#define Default 8

#define RunAlpha 0 // 0: no, 1: sí
#define RunMerge 0 // 0: no, 1: sí
#define RunQuick 1 // 0: no, 1: sí
#define RunAll 0 // 0: no, 1: sí

#define ARRAYNAME "arreglo.bin" // nombre del archivo binario

int nlogs = 0; // número de logs generados

static const char* FILE_ALPHA   = "arreglos_aridad.bin";

static void delete_temp_files()
{
    std::remove(FILE_ALPHA);
}

int findMaxArity() {
    constexpr int    MIN_ARITY  = 2;                 // assignment lower bound
    constexpr size_t WORD       = sizeof(uint64_t);  // element size in bytes

    /* 1· Memory‑capacity bound  ⌊ M/B ⌋ – 1 */
    int maxByMemory = (M * 1024 / B) - 1;          // M in MB, B in KB
    if (maxByMemory < MIN_ARITY)  maxByMemory = MIN_ARITY;

    /* 2· Pivot‑block bound      ⌊ blockBytes / elementSize ⌋ */
    int maxByBlock  = (B * 1024) / WORD;           // elements in one disk block
    if (maxByBlock  < MIN_ARITY)  maxByBlock  = MIN_ARITY;

    /* 3· Final upper limit = stricter of the two */
    return std::min(maxByMemory, maxByBlock);
}

int main() {
    
    std::cout << "Hola desde Docker!" << std::endl;
    std::signal(SIGINT,  [](int){ delete_temp_files(); std::exit(130); });
    std::signal(SIGTERM, [](int){ delete_temp_files(); std::exit(143); });
    
    // Primero, inicializo la clase que crea el arreglo
    // const char * filename = "arreglo.bin";
    // CrearArray creador(filename,M,Xtest);
    // const char * filename = "arreglo.bin";
    const char* envData = std::getenv("DATA_DIR");
    std::string baseData = envData ? envData : "";
    std::string fnameStr = baseData + "/" + ARRAYNAME;
    const char * fname = fnameStr.c_str();
    CrearArray creador(fname,M,Xtest);

    std::ofstream logFile("merge.log", std::ios::app); // append mode
    if (!logFile) {
        std::cerr << "Error abriendo merge.log" << std::endl;
        return 1;
    }

    // bool findAplpha;
    // std::cout << "¿Quieres encontrar la aridad? (X=" << Xtest << ") [1: sí, 0: no]: ";
    // std::cin >> findAplpha;
    int alfa;
    if (RunAlpha || RunAll) {
        std::cout << "Encontrando la aridad..." << std::endl;

        int maxArity = findMaxArity(); // 2 <= maxArity
        std::cout << "maxArity: " << maxArity << std::endl;


        auto start = std::chrono::system_clock::now();
        std::time_t start_time = std::chrono::system_clock::to_time_t(start);
        logFile << "Optimal Arity=" << 0 // Placeholder for initial log
                << ", M=" << M
                << ", X=" << Xtest
                << ", B=" << B
                << ", tiempo=" << std::put_time(std::localtime(&start_time), "%F %T")
                << ", startOfLog=" << nlogs
                << std::endl;

        alfa = findOptimalArity(maxArity,
                                "arreglos_aridad.bin",
                                M,
                                Xtest,     // X (array size factor)
                                B);        // block size KB

        auto end = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        logFile << "Optimal Arity=" << alfa
                    << ", M=" << M
                    << ", X=" << Xtest
                    << ", B=" << B
                    << ", maxArity=" << maxArity
                    << ", tiempo=" << std::put_time(std::localtime(&end_time), "%F %T")
                    << ", endOfLog=" << nlogs++
                    << std::endl;
        
        std::remove("arreglos_aridad.bin");
    } else {
        std::cout << "Aridad default: " << Default << std::endl;
        alfa = Default;
    }

    std::cout << "Optimal arity: " << alfa << std::endl;
    
    // Realizar pruebas con MergeSort
    if (RunMerge || RunAll) {
        std::cout << "Ejecutando MergeSort..." << std::endl;
        for (size_t Xi = 4; Xi <= 60; Xi+=4) {
            for (int j = 0; j < 5; ++j) {
                // Seteo el tamaño del arreglo
                creador.setX(Xi);
                // Calculo el tamaño del archivo en bytes
                size_t largo_archivo = static_cast<size_t>(M) * Xi * 1024 * 1024;
                // Creamos la secuencia de Xi * M numeros de 64 bits
                creador.crearArrayN();
                // Mergesort y guardar los resultados
                MergeSort mergesort(fname, alfa, largo_archivo, 0, B);
                // Concat time to 'merge.log' before and after MergeSort
                auto start = std::chrono::system_clock::now();
                std::time_t start_time = std::chrono::system_clock::to_time_t(start);
                logFile << "MergeSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << 0 // Placeholder for initial log
                        << ", tiempo=" << std::put_time(std::localtime(&start_time), "%F %T")
                        << ", startOfLog=" << nlogs
                        << std::endl;
                std::cout << "MergeSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << 0 // Placeholder for initial log
                        << ", tiempo=" << std::put_time(std::localtime(&start_time), "%F %T")
                        << ", startOfLog=" << nlogs
                << std::endl;
                int IOs_merge = mergesort.MergeSortN(M, B);
                auto end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                std::cout << "MergeSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << IOs_merge
                        << ", tiempo=" << std::put_time(std::localtime(&end_time), "%F %T")
                        << ", endOfLog=" << nlogs
                        << std::endl;        
                logFile << "MergeSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << IOs_merge
                        << ", tiempo=" << std::put_time(std::localtime(&end_time), "%F %T")
                        << ", endOfLog=" << nlogs++
                        << std::endl;
                // Imprimo la cantidad de IOs
                std::cout << "Usaron esta cantidad de IOs en MergeSort: " << IOs_merge << std::endl;
                // Finalmente, borro lo que había en el archivo.bin y sigo con la próxima secuencia
                // std::remove("arreglo.bin"); // Uncomment if you want to delete the file after each run
            }
        }
    }
    
    // Ahora, usando el alfa, realizamos QuickSort
    if (RunQuick || RunAll) {
        std::cout << "Ejecutando QuickSort..." << std::endl;
        for (size_t Xi = 40; Xi <= 60; Xi+=4) {
            for (int j = 0; j < 1; ++j) {
                // Seteo el tamaño del arreglo
                creador.setX(Xi);
                // Calculo el tamaño del archivo en bytes
                size_t largo_archivo = static_cast<size_t>(M) * Xi * 1024 * 1024;
                // Creamos la secuencia de Xi * M numeros de 64 bits
                creador.crearArrayN();
                // QuickSort y guardar los resultados
                QuickSort quicksort(fname, alfa, largo_archivo, 0, B);
                // Concat time to 'merge.log' before and after MergeSort
                auto start = std::chrono::system_clock::now();
                std::time_t start_time = std::chrono::system_clock::to_time_t(start);
                logFile << "QuickSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << 0 // Placeholder for initial log
                        << ", tiempo=" << std::put_time(std::localtime(&start_time), "%F %T")
                        << ", startOfLog=" << nlogs
                        << std::endl;
                std::cout << "QuickSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << 0
                        << ", tiempo=" << std::put_time(std::localtime(&start_time), "%F %T")
                        << ", startOfLog=" << nlogs
                        << std::endl;
                int IOs_quick = quicksort.QuickSortN(M, B);
                auto end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                std::cout << "QuickSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << IOs_quick
                        << ", tiempo=" << std::put_time(std::localtime(&end_time), "%F %T")
                        << ", endOfLog=" << nlogs
                        << std::endl;        
                logFile << "QuickSort: Xi=" << Xi
                        << ", M=" << M
                        << ", B=" << B
                        << ", IOs=" << IOs_quick
                        << ", tiempo=" << std::put_time(std::localtime(&end_time), "%F %T")
                        << ", endOfLog=" << nlogs++
                        << std::endl;
                // Imprimo la cantidad de IOs
                std::cout << "Usaron esta cantidad de IOs en QuickSort: " << IOs_quick << std::endl;
                // Finalmente, borro lo que había en el archivo.bin y sigo con la próxima secuencia
                // std::remove("arreglo.bin"); // Uncomment if you want to delete the file after each run
            }
        }
    }

    delete_temp_files();

    return 0;
}