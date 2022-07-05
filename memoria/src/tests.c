#include "memoria.h"

void test_inicializar_proceso(void){

    int i = 0, j = 0;
    tabla_segundo_nivel * tabla_segundo_nivel_pointer;
    entrada_segundo_nivel * entrada_segundo_nivel_pointer;

    pthread_create(&h3, NULL, hilo_swap, NULL);

    tabla_primer_nivel * tabla_pointer = inicializar_proceso(4, 80);

    //testeo dictionary
    void * temporal = dictionary_get(diccionario_tabla_pointers, "4");

    log_warning(logger, "tamanio del diccionario despues de crear la entrada: %d", dictionary_size(diccionario_tabla_pointers));

    log_warning(logger, "tabla_pointer: %p  dictionary_get: %p (tienen que ser iguales)", tabla_pointer, temporal);

    tabla_pointer = temporal;

    //testeo la tabla generada

    log_info(logger,"pid=%d, tamanio_lista:%d", tabla_pointer->pid, list_size(tabla_pointer->lista_de_tabla_segundo_nivel));

    for (i = 0; i < list_size(tabla_pointer->lista_de_tabla_segundo_nivel); i++){
        tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, i);
        log_info(logger, "tamanio_lista_segundo_nivel: %d", list_size(tabla_segundo_nivel_pointer->lista_de_entradas));

        for (j = 0; j < list_size(tabla_segundo_nivel_pointer->lista_de_entradas); j++){
            entrada_segundo_nivel_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, j);
            log_warning(logger, "numero marco: %d", entrada_segundo_nivel_pointer->numero_marco);
        }

    }

    //borro la entrada de diccionario para testear

    borrar_entrada_diccionario_tabla_pointers(4);

    log_warning(logger, "tamanio del diccionario despues de borrar la entrada: %d", dictionary_size(diccionario_tabla_pointers));

    sleep(60);
}