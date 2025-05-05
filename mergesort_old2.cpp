#include "mergesort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <tuple>

MergeSort::MergeSort(const char* filename,int alfa,size_t largo,int inicio,size_t B){
    this->filename = filename;
    this->alfa = alfa;
    this->largo = largo;
    this->inicio = inicio;
    this->B = B;
    this->hijos.resize(alfa);
    this->pos_merge_ = this->inicio;
}

const char * MergeSort::getFileName() const{
    return this->filename;
}

int MergeSort::getAlfa() const{
    return this->alfa;
}

size_t MergeSort::getLargo() const{
    return this->largo;
}
int MergeSort::getInicio() const{
    return this->inicio;
}

int MergeSort::MergeSortN(int M,size_t B) {
    int IOs = 0;
    std::ifstream archivo(filename, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo binario." << std::endl;
        return 1;
    }
    
    if(largo>M*1024*1024){
        for(int i = 0; i<this->alfa;++i){
            size_t largo_nuevo = largo/this->alfa;
            int start = i*(largo_nuevo/sizeof(uint64_t)) ;
            //El de este sea raiz, y que los demás MergeSort sean sus hojas
            MergeSort hijo(filename,this->alfa,largo_nuevo,start,B);
            this->hijos[i] = hijo;
            IOs+=hijo.MergeSortN(M,B);
        }
        //Es un cuarto, ya que uso la un cuarto para cada arreglo, y un medio para ambos juntos
        IOs+=unionHijos(M,B,archivo);
        archivo.close();
        std::cout << "El número de IOs para el MergeSort es: " << IOs << std::endl;
        return IOs;
    }
    else{
        //Lo ordeno aquí en "memoria princiapl"
        std::cout << "Ordenando en memoria principal" << std::endl;
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
 *  REFACTORED unionHijos                                        *
 *  – Lee y escribe EXACTAMENTE un bloque (B KB) por I/O.        *
 *  – Usa un min‑heap k‑way O(log α) para elegir el mínimo.       *
 *  – Mantiene un buffer_out de tamaño B para la escritura.       *
 *****************************************************************/
int MergeSort::unionHijos(int M, size_t B, std::ifstream& in) const {
    using Word = uint64_t;
    const size_t nums_per_blk = (B * 1024) / sizeof(Word);   // elementos/B
    const size_t buf_out_cap  = nums_per_blk;                // una B

    /*------------ 1. Buffers por hijo y punteros de progreso ----*/
    struct BufInfo {
        std::vector<Word> data;
        size_t len   = 0;     // # de palabras válidas en data
        size_t idx   = 0;     // cursor dentro de data
        size_t next;          // posición absoluta (en enteros) a leer
        size_t end;           // posición (excl.) del último entero del hijo
    };
    std::vector<BufInfo> child(this->alfa);

    auto child_end = [&](int i) {
        return this->hijos[i].getInicio() +
               this->hijos[i].getLargo() / sizeof(Word);
    };

    /*------------ 2. Rutina para cargar un bloque completo ------*/
    auto load_block = [&](int id) -> bool {
        BufInfo& b = child[id];
        if (b.next >= b.end) return false;            // nada más que leer
        size_t left_words  = b.end - b.next;
        b.len = std::min(nums_per_blk, left_words);
        b.data.resize(b.len);
        in.seekg(b.next * sizeof(Word), std::ios::beg);
        in.read(reinterpret_cast<char*>(b.data.data()),
                b.len * sizeof(Word));
        b.idx = 0;
        b.next += b.len;
        return true;                                 // se leyó algo
    };

    /*------------ 3. Inicializar buffers y heap -----------------*/
    std::priority_queue<
        std::tuple<Word,int>,                         // valor, hijo
        std::vector<std::tuple<Word,int>>,
        std::greater<> > heap;

    int IOs = 0;
    for (int i = 0; i < this->alfa; ++i) {
        child[i].next = this->hijos[i].getInicio();
        child[i].end  = child_end(i);
        if (load_block(i)) {                          // primera carga
            heap.emplace(child[i].data[0], i);
            ++IOs;                                    // 1 lectura B
        }
    }

    /*------------ 4. Merge k‑way -------------------------------*/
    std::vector<Word> out;
    out.reserve(buf_out_cap);

    auto flush_out = [&] {
        std::fstream outF(filename,
                          std::ios::in  | std::ios::out |
                          std::ios::binary);
        outF.seekp(/*this->inicio*/ 0 +                // TODO: offset real
                   (pos_merge_) * sizeof(Word), std::ios::beg);
        outF.write(reinterpret_cast<char*>(out.data()),
                   out.size() * sizeof(Word));
        ++IOs;                                        // 1 escritura B
        pos_merge_ += out.size();
        out.clear();
    };

    while (!heap.empty()) {
        auto [v, id] = heap.top();
        heap.pop();
        out.push_back(v);

        BufInfo& b = child[id];
        ++b.idx;                                     // avanzar cursor

        if (b.idx == b.len) {                        // se agotó buffer
            if (load_block(id)) {                    // recargar
                heap.emplace(b.data[0], id);
                ++IOs;
            }
        } else {
            heap.emplace(b.data[b.idx], id);
        }

        if (out.size() == buf_out_cap) flush_out();
    }
    if (!out.empty()) flush_out();                   // lo que reste
    return IOs;
}

int MergeSort::lectura_bloques(size_t B, int cantidad_lectura, std::vector<uint64_t> &buffer,std::ifstream &archivo,int pos) const{
    int lecturas_necesarias = (cantidad_lectura*sizeof(uint64_t))/(B*1024);
        for(int i = 0; i<lecturas_necesarias;i++){
            size_t cant_lectura_bloque = B*1024;
            //Caso borde
            if(i==lecturas_necesarias-1){
                cant_lectura_bloque = cantidad_lectura*sizeof(uint64_t) - i*cant_lectura_bloque;
            }
            archivo.read(reinterpret_cast<char*>(buffer.data()), cant_lectura_bloque);
        }
    return lecturas_necesarias;
}

int MergeSort::escritura_bloques(size_t B, int cantidad_escritura, std::vector<uint64_t> &buffer,std::fstream &archivo) const{
    int escrituras_necesarias = (cantidad_escritura*sizeof(uint64_t))/(B*1024);
    for(int i = 0; i<escrituras_necesarias;i++){
        size_t cant_escritura_bloque = B*1024;
        //Caso borde
        //if(i==escrituras_necesarias-1){
        //    cant_escritura_bloque = cantidad_escritura*sizeof(uint64_t) - i*cant_escritura_bloque;
        //}
        archivo.write(reinterpret_cast<char*>(buffer.data()), cant_escritura_bloque);
    }
    return escrituras_necesarias;
}