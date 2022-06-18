#include "memoria.h"

void *hilo_swap(void *arg){

    // Implementa un productor-consumidor con todas las instrucciones que debe hacer

    instruccion_swap *instruccion_pointer;
    
    while(1){
        sem_wait(&contador_cola_instrucciones_swap);

        pthread_mutex_lock(&mutex_cola_instrucciones_swap);
        instruccion_pointer = queue_pop(cola_instrucciones_swap);
        pthread_mutex_unlock(&mutex_cola_instrucciones_swap);

        sleep(RETARDO_SWAP); //retardo intencional para emular un swap real

        // Llamo a la instruccion correspondiente pasandole los parametros que correspondan
        
        //TODO: FIJARSE QUE PARAMETROS SE PASAN EN CADA FUNCION

        switch (instruccion_pointer->numero_instruccion){
            case CREAR_ARCHIVO_SWAP:
                crear_archivo_swap(instruccion_pointer->pid, instruccion_pointer->tamanio_proceso);
                break;
            case TRASLADAR_PAGINA_A_DISCO:
                trasladar_pagina_a_disco(instruccion_pointer->numero_marco, instruccion_pointer->pid);
                break;
            case TRASLADAR_PAGINA_A_MEMORIA:
                trasladar_pagina_a_memoria(instruccion_pointer->numero_marco, instruccion_pointer->pid);
                break;
            case TRASLADAR_PROCESO_A_DISCO:
                trasladar_proceso_a_disco(instruccion_pointer->pid);
                break;
            case TRASLADAR_PROCESO_A_MEMORIA:
                trasladar_proceso_a_memoria(instruccion_pointer->pid);
                break;
            case BORRAR_ARCHIVO_SWAP:
                borrar_archivo_swap(instruccion_pointer->pid);
                break;
            default:
                log_error(logger, "No existe el codigo de instruccion");
                break;
        }

        free(instruccion_pointer);
    }

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

void trasladar_pagina_a_disco(int32_t numero_marco, int32_t pid){

    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado
    void marco_temp[TAM_PAGINA]; // Marco temporal
    memcpy(marco_temp, marco, TAM_PAGINA); // Guardo el contenido del marco en el marco temporal

    FILE f;
    char swap_file[MAX_STRING_SIZE];
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    f = fopen(swap_file, "r+");

    fseek(f, /*TAM_PAGINA * NroDePagina*/, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fwrite(marco_temp, TAM_PAGINA, 1, f);

    fclose(f);
    // TODO
    // Busco la tabla primaria de este pid
    // Busco la tabla secundaria de esta pagina
    // Pongo el bit de presencia de esa pag en 0

    return;
}

void trasladar_pagina_a_memoria(int32_t numero_marco, int32_t pid){
    //TODO: IMPLEMENTAR
    int pag_de_proceso = /*Idem anterior, busco por el marco y pid la pag del proceso*/;
    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado

    FILE f;
    char swap_file[MAX_STRING_SIZE]; 
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    f = fopen(swap_file, "r+");

    fseek(f, /*TAM_PAGINA * NroDePagina*/, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fread(marco, TAM_PAGINA, 1, f);
    
    fclose(f);

    // TODO
    // Busco la tabla primaria de este pid
    // Busco la tabla secundaria de esta pagina
    // Pongo el bit de presencia de esa pag en 0

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

void borrar_archivo_swap(int32_t pid){

    char swap_file[MAX_STRING_SIZE];

    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);

    if(remove(swap_file) == 0){
        log_info(logger, "Se ha eliminado el archivo %s con exito.", swap_file);
    } else {
        log_error(logger, "error al eliminar el archivo swap: %s",swap_file);
    }

}
