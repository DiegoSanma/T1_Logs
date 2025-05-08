#include "quicksort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using Word = uint64_t;

QuickSort::QuickSort(const char* filename, int alfa, size_t largo, int inicio, size_t B) {
    this->filename = filename;
    this->alfa     = alfa;
    this->largo    = largo;
    this->inicio   = inicio;
    this->B        = B;
}

const char* QuickSort::getFileName() const {
    return filename;
}

int QuickSort::getAlfa() const {
    return alfa;
}

size_t QuickSort::getLargo() const {
    return largo;
}

int QuickSort::getInicio() const {
    return inicio;
}

// Main entry: if data <= M MB, sort in RAM; else partition externally
int QuickSort::QuickSortN(int M, size_t B) {
    size_t byteSize = largo;
    // (1) In-memory sort when small
    if (byteSize <= (size_t)M * 1024 * 1024) {
        std::ifstream in(filename, std::ios::binary);
        in.seekg(inicio * sizeof(Word), std::ios::beg);
        size_t count = byteSize / sizeof(Word);
        std::vector<Word> buffer(count);
        in.read(reinterpret_cast<char*>(buffer.data()), count * sizeof(Word));
        in.close();

        std::sort(buffer.begin(), buffer.end());

        std::fstream out(filename, std::ios::in | std::ios::out | std::ios::binary);
        out.seekp(inicio * sizeof(Word), std::ios::beg);
        out.write(reinterpret_cast<char*>(buffer.data()), count * sizeof(Word));
        out.close();

        // 1 read + 1 write of count words => count*sizeof(Word)/(B*1024) blocks each
        return 2 * ((int)count * sizeof(Word) / (B * 1024));
    }

    // (2) External partition when too large
    std::ifstream src(filename, std::ios::binary);
    if (!src.is_open()) {
        std::cerr << "Error opening file for partitioning." << std::endl;
        return 0;
    }
    int IOs = qsHijos(M, B, src);
    src.close();
    return IOs;
}

// External partitioning phase
int QuickSort::qsHijos(int M, size_t B, std::ifstream& src) const {
    // 0 · Compute block and window sizes
    const size_t WORDS_PER_BLK = (B * 1024) / sizeof(Word);
    const size_t WORDS_RAM     = (size_t)M * 1024 * 1024 / sizeof(Word);
    const size_t WIN_SIZE      = std::max(WORDS_PER_BLK, WORDS_RAM / (alfa + 1));
    size_t startIdx = inicio;
    size_t endIdx   = inicio + (largo / sizeof(Word));
    int IOs = 0;

    // 1 · Setup bucket buffers and files
    struct Bucket {
        std::vector<Word> buf;
        size_t written = 0;
        std::fstream tmp;
        std::string name;
    };
    std::vector<Bucket> bucket(alfa);
    const char* envDir = std::getenv("BUCKET_DIR");
    std::string baseDir = envDir ? envDir : "";
    for (int i = 0; i < alfa; ++i) {
        bucket[i].buf.reserve(WIN_SIZE);
        bucket[i].name = baseDir + "/bucket-" + std::to_string(i) + ".bin";
        bucket[i].tmp.open(bucket[i].name,
                           std::ios::in | std::ios::out | std::ios::binary);
        if (!bucket[i].tmp.is_open()) {
            bucket[i].tmp.open(bucket[i].name,
                               std::ios::in | std::ios::out |
                               std::ios::binary | std::ios::trunc);
        }
    }

    // 2 · Sample pivots from first block
    std::vector<Word> window(WORDS_PER_BLK);
    src.seekg(startIdx * sizeof(Word), std::ios::beg);
    src.read(reinterpret_cast<char*>(window.data()), WORDS_PER_BLK * sizeof(Word));
    IOs += 1;
    std::vector<Word> piv(alfa - 1);
    for (int i = 0; i < alfa - 1; ++i) piv[i] = window[i];
    std::sort(piv.begin(), piv.end());

    // 3 · Partition: read blocks, assign to buckets
    size_t next = startIdx;
    while (next < endIdx) {
        size_t remain = endIdx - next;
        size_t toRead = std::min(WORDS_PER_BLK, remain);
        src.seekg(next * sizeof(Word), std::ios::beg);
        src.read(reinterpret_cast<char*>(window.data()), toRead * sizeof(Word));
        IOs++;
        for (size_t j = 0; j < toRead; ++j) {
            Word v = window[j];
            int id = alfa - 1;
            for (int p = 0; p < alfa - 1; ++p) {
                if (v < piv[p]) { id = p; break; }
            }
            auto &b = bucket[id];
            b.buf.push_back(v);
            if (b.buf.size() == WIN_SIZE) {
                b.tmp.write(reinterpret_cast<char*>(b.buf.data()), WIN_SIZE * sizeof(Word));
                IOs += (int)((WIN_SIZE + WORDS_PER_BLK - 1) / WORDS_PER_BLK);
                b.written += WIN_SIZE;
                b.buf.clear();
            }
        }
        next += toRead;
    }
    // Flush remaining buffers
    for (int i = 0; i < alfa; ++i) {
        auto &b = bucket[i];
        if (!b.buf.empty()) {
            b.tmp.write(reinterpret_cast<char*>(b.buf.data()), b.buf.size() * sizeof(Word));
            IOs += (int)((b.buf.size() + WORDS_PER_BLK - 1) / WORDS_PER_BLK);
            b.written += b.buf.size();
            b.buf.clear();
        }
    }

    // 4 · Concatenate buckets into main file
    if (truncate(filename, 0) != 0) perror("truncate main file");
    std::fstream out(filename, std::ios::in | std::ios::out | std::ios::binary);
    size_t dst = startIdx;
    std::vector<Word> readBuf(WORDS_PER_BLK);
    for (int i = 0; i < alfa; ++i) {
        auto &b = bucket[i];
        b.tmp.seekg(0, std::ios::beg);
        size_t left = b.written;
        while (left) {
            size_t chunk = std::min(WORDS_PER_BLK, left);
            b.tmp.read(reinterpret_cast<char*>(readBuf.data()), chunk * sizeof(Word));
            out.seekp(dst * sizeof(Word), std::ios::beg);
            out.write(reinterpret_cast<char*>(readBuf.data()), chunk * sizeof(Word));
            IOs += 2;
            dst += chunk;
            left -= chunk;
        }
        b.tmp.close();
    }
    out.close();

    // 5 · Cleanup metadata, record counts
    std::vector<size_t> counts(alfa);
    for (int i = 0; i < alfa; ++i) counts[i] = bucket[i].written;
    bucket.clear();

    // 6 · Recursion on buckets
    size_t offset = startIdx;
    for (int i = 0; i < alfa; ++i) {
        size_t cnt = counts[i];
        if (cnt == 0) continue;
        QuickSort child(filename, alfa, cnt * sizeof(Word), offset, B);
        IOs += child.QuickSortN(M, B);
        offset += cnt;
    }

    return IOs;
}
