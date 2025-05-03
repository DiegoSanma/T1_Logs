#include "mergesort.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

MergeSort::MergeSort(const char* filename,int alfa,size_t largo,int inicio){
    this->filename = filename;
    this->alfa = alfa;
    this->largo = largo;
    this->inicio = inicio;
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

int MergeSort::MergeSortN(int M) const{
    int IOs = 0;
    std::ifstream archivo(filename, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo binario." << std::endl;
        return 1;
    }

    if(largo>M){
        for(int i = 0; i<this->alfa;++i){
            int start = i*this->alfa +1;
            size_t largo_nuevo = largo/this->alfa;
            //El de este sea raiz, y que los demás MergeSort sean sus hojas
            MergeSort hijo(filename,this->alfa,largo_nuevo,start);
            this->hijos[i] = hijo;
            IOs+=hijo.MergeSortN(M);
        }
        //Es un cuarto, ya que uso la un cuarto para cada arreglo, y un medio para ambos juntos
        IOs+=unionHijos(M,archivo);
        archivo.close();
        return IOs;
    }
    else{
        //Lo ordeno aquí en "memoria princiapl"
        int pos_final = inicio + largo;
        int cantidad = pos_final - inicio + 1;             // cuántos enteros quieres leer
        std::vector<uint64_t> buffer(cantidad);                   // buffer para guardarlos

        //Leo la cosa
        archivo.seekg(inicio * sizeof(uint64_t), std::ios::beg);   
        archivo.read(reinterpret_cast<char*>(buffer.data()), cantidad * sizeof(uint64_t));
        std::sort(buffer.begin(),buffer.end());
        archivo.close();
        //Dsp de ordenar, lo reescribo en el archivo
        std::ofstream archivoFuera(filename, std::ios::binary);
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

int MergeSort::unionHijos(int M,std::ifstream& archivo) const{
    int IOs = 0;
    //Debo unir los alfa hijos que son parte del MergeSort
    int tamaño_agregar = (M*1024*1024)/(this->alfa+1); //Tamaño que meto a memoria para mergear
    int cantidad_num = tamaño_agregar/sizeof(uint64_t);
    //Arreglo con los buffers que tienen los trozos de cada aridad del mergesort
    std::vector<uint64_t> buffer((this->alfa)*tamaño_agregar);
    //Arreglo con los punteros al inicio de sus arreglos 
    std::vector<uint64_t> inicios(this->alfa);
    //Buffer donde se escribe el resultado merged (tamaño M/(this->alfa +1))
    std::vector<uint64_t> buffer_final;
    //Leo cada trozo y lo guardo en buffer
    for(int i=0;i<this->alfa;++i){
        int inicio_actual = this->hijos[i].getInicio();
        inicios[i] = inicio_actual;
        archivo.seekg(inicios[i] * sizeof(uint64_t), std::ios::beg);
        archivo.read(reinterpret_cast<char*>(buffer.data()),cantidad_num*sizeof(uint64_t));
    }
    //Sumo alfa IOs
    IOs+=this->alfa;
    //Punteros para cada arreglo que se va a mergear
    std::vector<size_t> indices(this->alfa, 0);
    //Cantidad de trozos del arreglo subido por cada hijo
    std::vector<int> trozos_subidos(this->alfa,0);
    //Posición donde debo escribir cada buffer final en disco
    int pos_buffer_final = 0;
    //Vector que guarda si ya mande todos los valores de un arreglo
    //cant_num -> no es el ultimo, x -> esta leyendo el ultimo bloque con x numeros, -1 -> los leyó todos
    std::vector<int> ultimo(this->alfa,cantidad_num);
    while (true) {
        uint64_t min_val = UINT64_MAX;
        size_t min_idx = -1;
        int indice_arreglo_menor = -1;

        // Saco el minimo de cada uno y reviso el menor
        for (size_t i = 0; i < this->alfa; ++i) {
            //Solo si aún quedan elementos por revisar en este arreglo
            if(ultimo[i]!=-1){
                size_t indice_numero = (i * ultimo[i] +indices[i]);
                //Debo revisar si me pase, para agregar el otro trozo de ese arreglo
                if(indice_numero>=(i+1)*ultimo[i]){
                    //Aumento los trozos subidos y reinicio el indice para este bloque
                    trozos_subidos[i]++;
                    indices[i] = 0;
                    int pos_nueva = (inicios[i]+ultimo[i]*trozos_subidos[i]);
                    int cantidad_a_llevar = ultimo[i];
                    //Si ya estoy en el último bloque del arreglo a subir
                    if(pos_nueva+(ultimo[i])>=this->hijos[i+1].getInicio()){
                        //Debo llevar a memoria un bloque más pequeño, evitar repetir numeros
                        cantidad_a_llevar = this->hijos[i+1].getInicio()-pos_nueva;
                        //Aviso que estoy/ya revise el último bloque
                        if(ultimo[i]==cantidad_num){
                            ultimo[i]= cantidad_a_llevar;
                        }
                        else{
                            ultimo[i]= -1;
                        }
                    }
                    //Leo el siguiente bloque y lo guardo en su posición en el buffer
                    archivo.seekg(pos_nueva*sizeof(uint64_t));
                    archivo.read(reinterpret_cast<char*>(&buffer[i*tamaño_agregar]),cantidad_a_llevar*sizeof(uint64_t));
                    //Sumo un IO por leer
                    IOs+=1;
                    indice_numero = i*cantidad_num + indices[i];
                }
                uint64_t valor = buffer[indice_numero];
                //Si este es el menor, lo dejo y guardo tmb de que arreglo salió
                if(valor<min_val){
                    min_val = valor;
                    min_idx = indice_numero;
                    indice_arreglo_menor = i;
                }
            }
        }

        // Si no hay más elementos, terminamos (todos quedaron con ultimo[i]=-1)
        if (min_idx == -1) {
            //Escribo lo que quedo en el buffer final en disco
            std::ofstream archivoFuera(filename, std::ios::binary);
            if (!archivoFuera.is_open()) {
                std::cerr << "No se pudo abrir el archivo." << std::endl;
                std::exit(1);
            }
            //Lo escribo en en disco
            archivoFuera.seekp(inicio * sizeof(uint64_t), std::ios::beg);
            archivoFuera.write(reinterpret_cast<char*>(buffer_final.data()), buffer.size() * sizeof(uint64_t));
            //Sumo un IO por escribir
            IOs+=1;
            archivoFuera.close();
            break;
        }

        // Insertamos el valor mínimo en el resultado
        buffer_final.push_back(min_val);

        //Si se nos llenó el buffer_final, hay que escribirlo en disco para vaciarlo
        if(buffer_final.size()>=tamaño_agregar/sizeof(uint64_t)){             
            std::ofstream archivoFuera(filename, std::ios::binary);
            if (!archivoFuera.is_open()) {
                std::cerr << "No se pudo abrir el archivo." << std::endl;
                std::exit(1);
            }
            //Lo escribo en en disco
            archivoFuera.seekp(pos_buffer_final * sizeof(uint64_t), std::ios::beg);
            //Sumo un IO por escribir
            IOs+=1;
            archivoFuera.write(reinterpret_cast<char*>(buffer_final.data()), buffer.size() * sizeof(uint64_t));
            archivoFuera.close();
            //Cambio posicion de buffer para escribir en próximo bloque del disco (pos del numero, no bytes)
            pos_buffer_final += buffer.size();
            //Limpio el buffer para reutilizarlo
            buffer_final.clear();
        }
        //Modifico el indice del arreglo del que agregue
        indices[indice_arreglo_menor]++;

    }
    return IOs;
}
