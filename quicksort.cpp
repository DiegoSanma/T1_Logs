#include "quicksort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <tuple>
#include <string>
#include <cstdio>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        IOs+=qsHijos(M, B, archivo);
        archivo.close();
        return IOs;
    }
    else{
        int cantidad = largo/sizeof(uint64_t);
        std::vector<uint64_t> buffer(cantidad);

        archivo.seekg(inicio * sizeof(uint64_t), std::ios::beg);
        archivo.read(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        std::sort(buffer.begin(),buffer.end());
        archivo.close();

        std::fstream archivoFuera(filename, std::ios::in | std::ios::out |std::ios::binary);
        if (!archivoFuera.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
            std::exit(1);
        }
        archivoFuera.seekp(inicio * sizeof(uint64_t), std::ios::beg);
        archivoFuera.write(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        archivoFuera.close();
        return 2 * buffer.size()*sizeof(uint64_t) / (B*1024);
    }
}

int QuickSort::qsHijos(int M, size_t B, std::ifstream& src) const
{
    const char* envDir = std::getenv("BUCKET_DIR");
    std::string baseDir = envDir ? envDir : "";

    using Word = uint64_t;

    /* 0 · Constants and RAM layout --------------------------------------- */
    const size_t WORDS_PER_BLK = (B * 1024) / sizeof(Word);
    const size_t WORDS_RAM     = size_t(M) * 1024 * 1024 / sizeof(Word);
    const size_t WORDS_PER_WIN = std::max(WORDS_PER_BLK, WORDS_RAM / (alfa + 1));

    auto physBlocks = [&](size_t n) {
        return (n + WORDS_PER_BLK - 1) / WORDS_PER_BLK;
    };

    /* 1 · Destination windows (one per bucket) --------------------------- */
    struct WinDst {
        std::vector<Word> buf;
        size_t        written = 0;
        std::fstream  tmp;
        std::string   name;
    };

    std::vector<WinDst> bucket(alfa);
    for (int i = 0; i < alfa; ++i) {
        bucket[i].buf.reserve(WORDS_PER_WIN);
        bucket[i].name = baseDir + "/bucket-" + std::to_string(i) + ".bin";
        bucket[i].tmp.open(bucket[i].name,
            std::ios::in | std::ios::out | std::ios::binary);
        if (!bucket[i].tmp.is_open()) {
            bucket[i].tmp.open(bucket[i].name,
                std::ios::in | std::ios::out |
                std::ios::binary | std::ios::trunc);
        }
    }

    std::vector<Word> windowSrc(WORDS_PER_BLK);

    /* 2 · Choose α−1 pivots from the first block ------------------------ */
    src.seekg(inicio * sizeof(Word), std::ios::beg);
    src.read(reinterpret_cast<char*>(windowSrc.data()),
             WORDS_PER_BLK * sizeof(Word));
    std::vector<Word> piv(alfa - 1);
    for (int i = 0; i < alfa - 1; ++i)
        piv[i] = windowSrc[i];
    std::sort(piv.begin(), piv.end());

    size_t nextRead = inicio;
    size_t endAbs   = inicio + largo / sizeof(Word);
    int IOs = 1;

    auto resetBucket = [&](WinDst &w) {
        w.tmp.flush();
        if (truncate(w.name.c_str(), 0) != 0)
            perror("truncate");
        w.tmp.clear();
        w.tmp.seekp(0, std::ios::beg);
        w.written = 0;
    };
    for (int i = 0; i < alfa; ++i)
        resetBucket(bucket[i]);

    /* 3 · Partitioning loop: read parent blocks, assign to bucket ------- */
    auto flushBucket = [&](int id) {
        auto& w = bucket[id];
        w.tmp.write(reinterpret_cast<char*>(w.buf.data()),
                    w.buf.size() * sizeof(Word));
        IOs += physBlocks(w.buf.size());
        w.written += w.buf.size();
        w.buf.clear();
    };

    while (nextRead < endAbs) {
        size_t left = endAbs - nextRead;
        size_t nRead = std::min(WORDS_PER_BLK, left);
        src.seekg(nextRead * sizeof(Word), std::ios::beg);
        src.read(reinterpret_cast<char*>(windowSrc.data()),
                 nRead * sizeof(Word));
        IOs++;
        nextRead += nRead;

        for (size_t i = 0; i < nRead; ++i) {
            Word v = windowSrc[i];
            int id = alfa - 1;
            for (int p = 0; p < alfa - 1; ++p)
                if (v < piv[p]) { id = p; break; }
            auto& w = bucket[id];
            w.buf.push_back(v);
            if (w.buf.size() == WORDS_PER_WIN) flushBucket(id);
        }
    }

    for (int i = 0; i < alfa; ++i)
        if (!bucket[i].buf.empty()) flushBucket(i);

    /* 4 · Concatenate temp buckets into main file ------------------------ */
    if (truncate(filename, 0) != 0)
        perror("truncate main array file");
    std::fstream dst(filename,
                     std::ios::in | std::ios::out | std::ios::binary);
    size_t dstPos = inicio;
    std::vector<Word> readBuf(WORDS_PER_BLK);
    for (int i = 0; i < alfa; ++i) {
        auto &tmp = bucket[i].tmp;
        tmp.seekg(0, std::ios::beg);
        size_t left = bucket[i].written;
        while (left) {
            size_t n = std::min(WORDS_PER_BLK, left);
            tmp.read(reinterpret_cast<char*>(readBuf.data()),
                    n * sizeof(Word));
            dst.seekp(dstPos * sizeof(Word), std::ios::beg);
            dst.write(reinterpret_cast<char*>(readBuf.data()),
                    n * sizeof(Word));
            IOs += 2;
            dstPos += n;
            left  -= n;
        }
    }

    /* 4.5 · Teardown buffers & files before recursion -------------------- */
    std::vector<size_t> bucketSizes(alfa);
    for (int i = 0; i < alfa; ++i) {
        bucketSizes[i] = bucket[i].written * sizeof(Word);
        bucket[i].buf.clear();
        bucket[i].buf.shrink_to_fit();
        bucket[i].tmp.close();
    }
    bucket.clear();

    /* 5 · Recursive quicksort on each bucket ----------------------------- */
    size_t childStart = inicio;
    for (int i = 0; i < alfa; ++i) {
        size_t childLen = bucketSizes[i];
        if (childLen == 0) continue;
        QuickSort child(filename, alfa, childLen, childStart, B);
        IOs += child.QuickSortN(M, B);
        childStart += bucketSizes[i] / sizeof(Word);
    }
    return IOs;
}
