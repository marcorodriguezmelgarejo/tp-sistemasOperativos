#include "memoria.h"

int32_t elegir_marco_libre(tabla_primer_nivel* tabla_pointer){
    
    /*
        Devuelve el numero de un marco libre en el cual colocar una pagina que se va a traer desde disco.
        Elige el primero que vea disponible.
        Retorna -1 en caso de no haber mas memoria disponible.
    */

    int i = 0;
    int tamanio_bitarray = bitarray_get_max_bit(marcos_libres);
    int marco_elegido = -1;
    
    for(i = 0; i < tamanio_bitarray; i++){
        
        if (bitarray_test_bit(marcos_libres, i) == false){
            marco_elegido = i;
            break;
        }
    }

    return marco_elegido;
}

void marcar_marcos_como_libres_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Marca todos los marcos ocupados por el proceso como libres
    */

    int32_t i = 0, j = 0;
    tabla_segundo_nivel * tabla_segundo_nivel_pointer;
    entrada_segundo_nivel * entrada_segundo_nivel_pointer;
    
    for(i = 0; i < tabla_pointer->cantidad_entradas; i++){
        tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, i);
        
        for (j = 0; j < tabla_segundo_nivel_pointer->cantidad_entradas; j++){
            entrada_segundo_nivel_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, j);

            if (entrada_segundo_nivel_pointer->presencia == true){
                marcar_marco_como_libre(entrada_segundo_nivel_pointer->numero_marco);
            }
        }
    }
}

void marcar_marco_como_libre(int32_t numero_marco){
    bitarray_clean_bit(marcos_libres, numero_marco);
}

void marcar_marco_como_ocupado(int32_t numero_marco){
    bitarray_set_bit(marcos_libres, numero_marco);
}