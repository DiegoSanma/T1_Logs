#include "quicksort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <tuple>
#include <string>
#include <cstdio>

QuickSort::QuickSort(const char* filename,int alfa,size_t largo,int inicio,size_t B){
    this->filename = filename;
    this->alfa = alfa;
    this->largo = largo;
    this->inicio = inicio;
    this->B = B;
    this->hijos.resize(alfa);
    this->pos_qs_ = this->inicio;
}

const char * QuickSort::getFileName() const{
    return this->filename;
}

int QuickSort::getAlfa() const{
    return this->alfa;
}

size_t QuickSort::getLargo() const{
    return this->largo;
}
int QuickSort::getInicio() const{
    return this->inicio;
}

int QuickSort::QuickSortN(int M,size_t B) {
    int IOs = 0;
    std::ifstream archivo(filename, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo binario." << std::endl;
        return 1;
    }
    //Si aún no puedo mandar todo a memoria, voy separando el quicksort
    if(largo>M*1024*1024){
        std::cout << "Separo en hijos" << std::endl;
        IOs+=qsHijos(M, B, archivo);
        archivo.close();
        std::cout << "El número de IOs para el QuickSort es: " << IOs << std::endl;
        return IOs;
    }
    else{
        //Lo ordeno aquí en "memoria princiapl (no necesario hacerlo como quicksort)"
        // std::cout << "Ordenando en memoria principal" << std::endl;
        std::cout << "Ordeno en memoria" << std::endl;
        int pos_final = this->inicio + largo/sizeof(uint64_t);
        int cantidad = pos_final - this->inicio + 1;             // cuántos enteros quieres leer
        std::vector<uint64_t> buffer(cantidad);                   // buffer para guardarlos

        //Leo la cosa
        archivo.seekg(inicio * sizeof(uint64_t), std::ios::beg);  
        archivo.read(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        std::sort(buffer.begin(),buffer.end());
        archivo.close();
        //Dsp de ordenar, lo reescribo en el archivo
        std::fstream archivoFuera(filename, std::ios::in | std::ios::out |std::ios::binary);
        if (!archivoFuera.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
            std::exit(1);
        }
        //Lo escribo en donde estaba antes los número desordenados
        archivoFuera.seekp(inicio * sizeof(uint64_t), std::ios::beg);
        archivoFuera.write(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        archivoFuera.close();
        //Sumo dos IOs, uno por leer y otro por escribir el bloque de tamaño <=M
        return 2;
    }
}

/*****************************************************************
 *  qsHijos  —  External quick‑sort partitioning phase            *
 *                                                               *
 *  RAM model                                                    *
 *  ──────────────────────────────────────────────────────────── *
 *   • 1  source window  (WORD_PER_BLK)                          *
 *   • α  destination windows (each WORDS_PER_WIN)               *
 *     WORDS_PER_WIN = max(WORDS_PER_BLK, M/(α+1)/8)             *
 *   Total ≤ M bytes.                                            *
 *****************************************************************/
int QuickSort::qsHijos(int M, size_t B, std::ifstream& src) const
{
    using Word = uint64_t;

    /* 0 · Derived sizes ----------------------------------------------------*/
    const size_t WORDS_PER_BLK = (B * 1024) / sizeof(Word);
    const size_t WORDS_RAM     = size_t(M) * 1024 * 1024 / sizeof(Word);
    const size_t WORDS_PER_WIN = std::max(WORDS_PER_BLK,
                                          WORDS_RAM / (alfa + 1));

    auto physBlocks = [&](size_t n)
        { return (n + WORDS_PER_BLK - 1) / WORDS_PER_BLK; };

    /* 1 · Windows ----------------------------------------------------------*/
    struct WinDst {
        std::vector<Word> buf;          // capacity WORDS_PER_WIN
        size_t written = 0;             // numbers already flushed
        std::ofstream tmp;              // temp file handle
    };
    std::vector<WinDst> bucket(alfa);               // dst windows
    for (int i = 0; i < alfa; ++i) {
        bucket[i].buf.reserve(WORDS_PER_WIN);
        bucket[i].tmp.open("bucket-" + std::to_string(i) + ".bin",
                           std::ios::binary | std::ios::trunc);
    }

    std::vector<Word> windowSrc(WORDS_PER_BLK);     // src window

    /* 2 · Choose α‑1 pivots  (simplest: first block, evenly spaced) --------*/
    src.seekg(inicio * sizeof(Word), std::ios::beg);
    src.read(reinterpret_cast<char*>(windowSrc.data()),
             WORDS_PER_BLK * sizeof(Word));
    std::vector<Word> piv(alfa - 1);
    for (int i = 0; i < alfa - 1; ++i)
        piv[i] = windowSrc[i];          // could sample better
    std::sort(piv.begin(), piv.end());

    size_t nextRead = inicio;           // absolute cursor in file
    size_t endAbs   = inicio + largo / sizeof(Word);
    int IOs = 1;                        // first read above

    /* 3 · Partition loop ---------------------------------------------------*/
    auto flushBucket = [&](int id) {
        auto& w = bucket[id];
        w.tmp.write(reinterpret_cast<char*>(w.buf.data()),
                    w.buf.size() * sizeof(Word));
        IOs += physBlocks(w.buf.size());
        w.written += w.buf.size();
        w.buf.clear();
    };

    while (nextRead < endAbs) {
        /* load one physical block from parent run */
        size_t left = endAbs - nextRead;
        size_t nRead = std::min(WORDS_PER_BLK, left);
        src.seekg(nextRead * sizeof(Word), std::ios::beg);
        src.read(reinterpret_cast<char*>(windowSrc.data()),
                 nRead * sizeof(Word));
        IOs++;                          // one physical read
        nextRead += nRead;

        /* distribute into buckets */
        for (size_t i = 0; i < nRead; ++i) {
            Word v = windowSrc[i];
            int id = alfa - 1;          // default bucket: greatest
            for (int p = 0; p < alfa - 1; ++p)
                if (v < piv[p]) { id = p; break; }

            auto& w = bucket[id];
            w.buf.push_back(v);
            if (w.buf.size() == WORDS_PER_WIN) flushBucket(id);
        }
    }

    /* flush remaining partial buffers */
    for (int i = 0; i < alfa; ++i)
        if (!bucket[i].buf.empty()) flushBucket(i);

    /* 4 · Concatenate all bucket files back into parent file ---------------*/
    std::fstream dst(filename,
                     std::ios::in | std::ios::out | std::ios::binary);
    size_t dstPos = inicio;
    for (int i = 0; i < alfa; ++i) {
        bucket[i].tmp.close();
        std::ifstream tmp("bucket-" + std::to_string(i) + ".bin",
                          std::ios::binary);
        size_t left = bucket[i].written;
        while (left) {
            size_t n = std::min(WORDS_PER_BLK, left);
            tmp.read(reinterpret_cast<char*>(windowSrc.data()),
                     n * sizeof(Word));
            dst.seekp(dstPos * sizeof(Word), std::ios::beg);
            dst.write(reinterpret_cast<char*>(windowSrc.data()),
                      n * sizeof(Word));
            IOs += 2;                   // read temp + write parent
            dstPos += n;  left -= n;
        }
        tmp.close();
        std::remove(("bucket-" + std::to_string(i) + ".bin").c_str());
    }

    /* 5 · Recursive quick‑sort calls --------------------------------------*/
    size_t childStart = inicio;
    for (int i = 0; i < alfa; ++i) {
        size_t childLen = bucket[i].written * sizeof(Word);
        if (childLen == 0) continue;    // empty bucket
        QuickSort child(filename, alfa, childLen, childStart, B);
        IOs += child.QuickSortN(M, B);
        childStart += bucket[i].written;
    }
    return IOs;
}