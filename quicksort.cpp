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
        qsHijos(M, B, archivo);
        archivo.close();
        std::cout << "El número de IOs para el QuickSort es: " << IOs << std::endl;
        return IOs;
    }
    else{
        //Lo ordeno aquí en "memoria princiapl (no necesario hacerlo como quicksort)"
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

int QuickSort::qsHijos(int M, size_t B, std::ifstream& in) const{
    using Word = uint64_t;
    const size_t NUMS_PER_BLK = (B * 1024) / sizeof(Word);   // # elementos en B KB

    /********** 1·Estructura auxiliar por hijo ************************/
    struct Window {
        std::vector<Word> buf;   // un bloque en RAM
        size_t len  = 0;         // cuántos números válidos hay en buf
        size_t idx  = 0;         // posición del “próximo” dentro de buf
        size_t next = 0;         // pos. absoluta del siguiente número a leer
        size_t end  = 0;         // pos. (excl.) donde termina el hijo
        size_t cant = 0;         // cantidad de bloque subidos en este subarreglo
        size_t ini = 0;          // pos global donde inicia el subarreglo en el arreglo A
    };
    /*****OJO ******/
    //La estructura Window se usa distinto según los indices del vector win
    //Para win[0] -> buffer que trae a memoria los numeros del arreglo
    //Para el resto -> buffer que almacena los valores que van a ir en cada subarreglo 
    std::vector<Window> win(alfa+1);
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
    int IOs = 0;
    /*********3 Función que escribe la ventana[i] en la fila i disco */
    auto flush_out = [&](int id) {        // escribe win[i] en disco
        std::ofstream outF("bloques" + std::to_string(id) + ".bin", std::ios::binary | std::ios::app);
        outF.seekp(win[id].next * sizeof(Word), std::ios::beg);
        outF.write(reinterpret_cast<char*>(win[id].buf.data()),
                   win[id].buf.size() * sizeof(Word));
        ++IOs;                    // 1 I/O por bloque escrito
        pos_qs_ += win[id].buf.size(); // avanzar su cursor global
        win[id].len += win[id].buf.size();
        win[id].cant++;
        win[id].buf.clear();
    };
    /*****************4 Elijo un bloque al azar para sacar los pivotes y ordenarlos **********/
    std::vector<Word> pivotes_elegidos;
    //win[0] guarda los bloques que se van leyendo para asignarlos al subarreglo correcto
    win[0].next = inicio;
    win[0].end = largo/sizeof(uint64_t);
    if(load_block(0)){
        for(int i = 0;i<this->alfa-1;i++){
            pivotes_elegidos[i] = win[0].buf[i];
        }
        std::sort(pivotes_elegidos.begin(),pivotes_elegidos.end());
        win[0].idx+=this->alfa-1;
        IOs++;
    };

    /*********5 Particiono recursivamente los alfa arreglos según los pivotes */
    //Parto con el bloque q ya envie (para no repetir los pivotes que ya elegi)
    Window &w = win[0];
    while(w.idx<w.len){
        Word valor = w.buf[w.idx];
        for(int i=0;i<this->alfa-1;i++){
            if(valor<pivotes_elegidos[i]){
                //Guardo el numero en el buffer donde va según el pivote
                win[i+1].buf.push_back(valor);
                break;
            }
        }
        w.idx++;  //Avanzo en esta ventana
    }
    //Subo el siguiente bloque a win[0]
    load_block(0);
    while(true){
        Word valor = w.buf[w.idx];
        int subarreglo = -1;
        for(int i=0;i<this->alfa-1;i++){
            if(valor<pivotes_elegidos[i]){
                //Guardo el numero en el buffer donde va según el pivote
                win[i+1].buf.push_back(valor);
                subarreglo = i+1;
                break;
            }
        }
        Window &w_usada = win[subarreglo];
        w_usada.idx++; //Avanzo en este subarreglo
        w.idx++;  //Avanzo en los bloques de numeros

        //Si ya se lleno el RAM este subarreglo, debo escribirlo en disco
        if(w_usada.buf.size()>=NUMS_PER_BLK){
            flush_out(subarreglo);
        }
        if(w.idx==w.len){
            //Se acabaron los números por revisar
            if(!load_block(0)){
                break;
            }
        }
    }
    for(int i =0;i<this->alfa;i++){
        flush_out(i+1);
    }

    /*****6 Concateno los bloques que escribi en los archivos separados */
    std::ofstream archivo_final(filename, std::ios::binary);
    for(int i = 0; i<this->alfa+1;i++){
        //Leo los bloques que están en el archivo temporal de cada subarreglo
        std::ifstream archivo_i("bloques" + std::to_string(i) + ".bin", std::ios::binary );
        //Guardo la posición inicial del subarreglo
        win[i].ini = pos_qs_;
        for(int j = 0;j<win[i+1].cant;i++){
            archivo_i.read(reinterpret_cast<char*>(win[i].buf.data()), NUMS_PER_BLK * sizeof(Word));
            archivo_final.seekp(pos_qs_*sizeof(Word));
            archivo_final.write(reinterpret_cast<char*>(win[i].buf.data()), win[i].buf.size() * sizeof(Word));
            pos_qs_+=win[i].buf.size();
            win[i].buf.clear();
        }
        archivo_i.close();
        //Borro el archivo temporal que tenía los bloques
        std::string arch_temporal_nombre = "bloques" + std::to_string(i) + ".bin";
        std::remove(arch_temporal_nombre.c_str());
    }
    archivo_final.close();
    /****6 LLamo recursivamente quicksort para todos los subarreglos */
    for(int i=0;i<this->alfa;i++){
        //El inicio es donde termina el anterior+1
        size_t inicio_subarreglo = win[i].ini;
        QuickSort hijo(filename,alfa,win[i+1].len,inicio_subarreglo,B);
        IOs+=hijo.QuickSortN(M,B);
    }
    return IOs;
}