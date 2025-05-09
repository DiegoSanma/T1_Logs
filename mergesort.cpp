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
            int start = inicio + i*(largo_nuevo/sizeof(uint64_t));
            //El de este sea raiz, y que los demás MergeSort sean sus hojas
            MergeSort hijo(filename,this->alfa,largo_nuevo,start,B);
            this->hijos[i] = hijo;
            IOs+=hijo.MergeSortN(M,B);
        }
        //Es un cuarto, ya que uso la un cuarto para cada arreglo, y un medio para ambos juntos
        IOs+=unionHijos(M,B,archivo);
        archivo.close();
        // std::cout << "El número de IOs para el MergeSort es: " << IOs << std::endl;
        return IOs;
    }
    else{
        //Lo ordeno aquí en "memoria princiapl"
        // std::cout << "Ordenando en memoria principal" << std::endl;
        int pos_final = this->inicio + largo/sizeof(uint64_t);
        int cantidad = pos_final - this->inicio;             // cuántos enteros quieres leer
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
        return 2 * buffer.size()*sizeof(uint64_t) / (B*1024);
    }
}

/*****************************************************************
 *  unionHijos  —  Fusiona los α hijos de este nodo               *
 *                                                               *
 *  Cambios clave                                                *
 *  ──────────────────────────────────────────────────────────── *
 *  [FIX‑RAM]  Cada hijo ahora usa ≈ M/(α+1) bytes de ventana     *
 *             en lugar de 1 bloque (4 KiB).                     *
 *  [FIX‑IO]   Contador de IOs basado en bloques físicos leídos   *
 *             o escritos (ceil(nWords / wordsPerBlock)).        *
 *  Parametros                                                    *
 *     M  ·  Memoria principal disponible en MB                  *
 *     B  ·  Tamaño de bloque de disco en KB                     *
 *     in ·  ifstream ya abierto (pos ≈ inicio del nodo)          *
 *****************************************************************/
int MergeSort::unionHijos(int M, size_t B, std::ifstream& in) const
{
    using Word = uint64_t;
    
     std::cout << "Unión de hijos" << std::endl;
    
    /*─────────── 0 · Constantes de bloque y RAM disponible ─────────*/
    const size_t WORDS_PER_BLK = (B * 1024) / sizeof(Word);      // 4 KiB→512 W
    const size_t WORDS_RAM     = (size_t)M * 1024 * 1024 / sizeof(Word);
    size_t WORDS_PER_WIN = WORDS_RAM / (alfa + 1);         // [FIX‑RAM]
    WORDS_PER_WIN = std::max(WORDS_PER_WIN, WORDS_PER_BLK);   // ← FIX

    auto blocks = [&](size_t n) {                // [FIX‑IO] helper
        return (n + WORDS_PER_BLK - 1) / WORDS_PER_BLK;
    };

    /********** 1 · Estructura “ventana” *****************************/
    struct Win {
        std::vector<Word> buf;      // capacidad fija = WORDS_PER_WIN
        size_t len=0, idx=0;        // len: # válidos, idx: cursor
        size_t next=0, end=0;       // rango absoluto pendiente
    };
    std::vector<Win> win(alfa);
    for (auto& w : win) w.buf.resize(WORDS_PER_WIN);             // [FIX‑RAM]

    /********** 2 · Carga de bloques (puede leer >1 bloque) **********/
    auto load = [&](int id) -> int {          // devuelve # IOs
        Win& w = win[id];
        if (w.next >= w.end) {w.len = 0; return 0;}        // no queda nada
        size_t left = w.end - w.next;
        w.len = std::min(WORDS_PER_WIN, left);
        /* leer en tandas de WORDS_PER_BLK hasta llenar ventana */
        size_t got = 0, ios = 0;
        while (got < w.len) {
            size_t now = std::min(WORDS_PER_BLK, w.len - got);
            in.seekg((w.next + got) * sizeof(Word), std::ios::beg);
            in.read(reinterpret_cast<char*>(w.buf.data() + got),
                    now * sizeof(Word));
            got += now;  ++ios;
        }
        w.idx = 0;
        w.next += w.len;
        return ios;                         // [FIX‑IO]
    };

    /********** 3 · Heap inicial                                     */
    using Tup = std::tuple<Word,int>;       // valor, idHijo
    std::priority_queue<Tup,std::vector<Tup>,std::greater<Tup>> heap;

    int IOs = 0;
    for (int i=0;i<alfa;++i) {
        win[i].next = hijos[i].getInicio();
        win[i].end  = win[i].next + hijos[i].getLargo()/sizeof(Word);
        IOs += load(i);                                     // [FIX‑IO]
        if (win[i].len) heap.emplace(win[i].buf[0], i);
    }

    /********** 4 · Buffer de salida del mismo tamaño que una ventana*/
    std::vector<Word> out;
    out.reserve(WORDS_PER_WIN);                              // [FIX‑RAM]

    std::fstream outF("arch_temporal.bin",
                      std::ios::in|std::ios::out|std::ios::binary); // 1 fd
    auto flush = [&] {                     // escribe y cuenta IOs
        size_t written = 0;
        while (written < out.size()) {
            size_t now = std::min(WORDS_PER_BLK, out.size() - written);
            outF.seekp((pos_merge_ + written) * sizeof(Word),
                       std::ios::beg);
            outF.write(reinterpret_cast<char*>(out.data()+written),
                        now * sizeof(Word));
            written += now;
            ++IOs;                                         // [FIX‑IO]
        }
        pos_merge_ += out.size();
        out.clear();
    };

    /********** 5 · Merge k‑way **************************************/
    while (!heap.empty()) {
        auto [v,id] = heap.top(); heap.pop();  //Obtengo el mínimo de los hijos
        out.push_back(v); //LO agrego a mi buffer de salida

        Win& w = win[id];
        if (w.idx + 1 == w.len) {                 // ventana agotada 
            IOs += load(id);                    // recarga (puede ser 0)
            if (w.len) heap.emplace(w.buf[0], id);
        } else {
            ++w.idx;
            heap.emplace(w.buf[w.idx], id);
        }

        if (out.size() == WORDS_PER_WIN) flush();
    }
    if (!out.empty()) flush();
    /**********6 · Add the temporary file to the disk*/

    int copied = 0;
    int cantidad = largo/sizeof(uint64_t);
    std::fstream outOficial(filename,
                      std::ios::in|std::ios::out|std::ios::binary);
    while(copied<cantidad){
        size_t now = std::min(WORDS_PER_BLK,static_cast<size_t>(cantidad-copied));
        outF.seekg((inicio+copied)*sizeof(Word),std::ios::beg);
        outF.read(reinterpret_cast<char*>(out.data()+copied),
                        now * sizeof(Word));
        outOficial.seekp((inicio+copied)*sizeof(Word),std::ios::beg);
        outOficial.write(reinterpret_cast<char*>(out.data()+copied),
                        now * sizeof(Word));
        copied+=now;
        IOs+=2;                
        
    }
    outF.close();
    outOficial.close();
    std::remove("arch_temporal.bin");
    return IOs;
}
