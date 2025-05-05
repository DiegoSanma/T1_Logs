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
        // std::cout << "Ordenando en memoria principal" << std::endl;
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
 *  unionHijos  —  Fusiona los α hijos de este nodo               *
 *  Param:                                                        *
 *     M  ·  Memoria principal disponible en MB (solo para cálcs) *
 *     B  ·  Tamaño de bloque en KB                               *
 *     in ·  flujo ya abierto en modo lectura (ifstream)          *
 *  Devuelve: # de I/Os hechos solo en esta fusión                *
 *****************************************************************/
int MergeSort::unionHijos(int /*M*/, size_t B, std::ifstream& in) const
{
    using Word = uint64_t;
    const size_t NUMS_PER_BLK = (B * 1024) / sizeof(Word);   // # elementos en B KB

    /********** 1·Estructura auxiliar por hijo ************************/
    struct Window {
        std::vector<Word> buf;   // un bloque en RAM
        size_t len  = 0;         // cuántos números válidos hay en buf
        size_t idx  = 0;         // posición del “próximo” dentro de buf
        size_t next = 0;         // pos. absoluta del siguiente número a leer
        size_t end  = 0;         // pos. (excl.) donde termina el hijo
    };
    std::vector<Window> win(alfa);

    /********** 2·Función lambda: leer un bloque **********************/
    auto load_block = [&](int id) -> bool {
        Window &w = win[id];
        if (w.next >= w.end) return false;            // nada más que leer
        size_t left  = w.end - w.next;                // cuántos faltan
        w.len  = std::min(NUMS_PER_BLK, left);
        w.buf.resize(w.len);

        in.seekg(w.next * sizeof(Word), std::ios::beg);
        in.read(reinterpret_cast<char*>(w.buf.data()), w.len * sizeof(Word));

        w.idx  = 0;            // reiniciar cursor interno
        w.next += w.len;       // avanzar posición absoluta
        return true;           // se leyó algo
    };

    /********** 3·Inicializar heap con el 1er bloque de cada hijo *****/
    using Tuple = std::tuple<Word,int>;               // valor, idHijo
    std::priority_queue<
        Tuple, std::vector<Tuple>, std::greater<Tuple>> heap;

    int IOs = 0;
    for (int i = 0; i < alfa; ++i) {
        win[i].next = hijos[i].getInicio();           // inicio absoluto
        win[i].end  = win[i].next +                   // fin exclusivo
                      hijos[i].getLargo()/sizeof(Word);
        if (load_block(i)) {      // ← 1 I/O por hijo si tiene datos
            heap.emplace(win[i].buf[0], i);
            ++IOs;
        }
    }

    /********** 4·Buffer de salida de EXACTO un bloque ***************/
    std::vector<Word> out;
    out.reserve(NUMS_PER_BLK);

    auto flush_out = [&] {        // escribe out en disco
        std::fstream outF(filename,
                          std::ios::in | std::ios::out | std::ios::binary);
        outF.seekp(pos_merge_ * sizeof(Word), std::ios::beg);
        outF.write(reinterpret_cast<char*>(out.data()),
                   out.size() * sizeof(Word));
        ++IOs;                    // 1 I/O por bloque escrito
        pos_merge_ += out.size(); // avanzar cursor global
        out.clear();
    };

    /********** 5·Merge k‑way ***************************************/
    while (!heap.empty()) {
        auto [val, id] = heap.top(); heap.pop();
        out.push_back(val);

        Window &w = win[id];
        ++w.idx;                                // avanzar en su buffer
        if (w.idx == w.len) {                   // ¿se agotó?
            if (load_block(id)) {               // recarga
                heap.emplace(w.buf[0], id);
                ++IOs;                          // cuenta la lectura
            }
        } else {
            heap.emplace(w.buf[w.idx], id);     // aún queda en RAM
        }

        if (out.size() == NUMS_PER_BLK) flush_out();
    }
    if (!out.empty()) flush_out();              // resto final
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