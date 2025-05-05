#include "crear_array.h"
#include <iostream>

int main(){
    //Primero, inicializo la clase que crea el arreglo
    CrearArray creador("arreglo.bin",50,4);
    std::cout << "Hola desde Docker!" << std::endl;
    for(int i=4;i<=60;i+=4){
        creador.setX(i);
        for(int j=0;j<5;++j){
            //Primero creamos la secuencia de i * M numeros de 64 bits
            creador.crearArrayN();
            //Dsp tenemos que hacerle mergesort y quicksort, y guardar los resultados
            //Finalmente, borro lo que había en el archivo.bin y sigo con la próxima secuencia
        
        }

    }
    std::cout << "Hola desde Docker2!" << std::endl;

    return 0;
}