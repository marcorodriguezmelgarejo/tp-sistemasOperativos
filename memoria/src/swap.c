#include "memoria.h"

void *hilo_swap(void *arg){
    //TODO: IMPLEMENTAR

    //implementa un productor-consumidor con todas las lecturas o escrituras que debe hacer

    
    return NULL;
}

void crear_archivo_swap(int32_t pid, int32_t tamanio_proceso){
    /*
        Crea un archivo con el nombre <pid>.swap y escribe ceros con cantidad 'tamanio_proceso'
    */

    char filename[MAX_STRING_SIZE];
    FILE * archivo_swap;
    char cero = 0;
    
    sprintf(filename, "%s/%d.swap", PATH_SWAP, pid);
    
    if ((archivo_swap = fopen(filename, "w")) == NULL){
        log_error(logger, "Error en 'crear_archivo_swap()': no se pudo crear el archivo swap");
        return;
    }

    fwrite(&cero, sizeof cero, tamanio_proceso, archivo_swap);

    fclose(archivo_swap);

    return;
}

void trasladar_pagina_a_disco(int32_t numero_marco){
    //TODO: IMPLEMENTAR
    return;
}

void trasladar_pagina_a_memoria(int32_t pid){
    //TODO: IMPLEMENTAR
    return;
}

void trasladar_proceso_a_disco(int32_t pid){
    //TODO: IMPLEMENTAR
    return;
}

void trasladar_proceso_a_memoria(int32_t pid){
    //TODO: IMPLEMENTAR
    return;
}
