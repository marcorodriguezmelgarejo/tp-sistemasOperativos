#include "memoria.h"

/*

    Funciones testeadas:
    crear_archivo_swap()

*/


void *hilo_swap(void *arg){

    // Implementa un productor-consumidor con todas las instrucciones que debe hacer

    instruccion_swap *instruccion_pointer;
    
    while(1){
        sem_wait(&contador_cola_instrucciones_swap);

        pthread_mutex_lock(&mutex_cola_instrucciones_swap);
        instruccion_pointer = queue_pop(cola_instrucciones_swap);
        pthread_mutex_unlock(&mutex_cola_instrucciones_swap);

        usleep(milisegundos_a_microsegundos(RETARDO_SWAP)); //retardo intencional para emular un swap real

        // Llamo a la instruccion correspondiente pasandole los parametros que correspondan

        switch (instruccion_pointer->numero_instruccion){
            case CREAR_ARCHIVO_SWAP:
                crear_archivo_swap(instruccion_pointer->pid, instruccion_pointer->tamanio_proceso);
                break;
            case TRASLADAR_PAGINA_A_DISCO:
                trasladar_pagina_a_disco(instruccion_pointer->pid, instruccion_pointer->numero_pagina, instruccion_pointer->numero_marco);
                break;
            case TRASLADAR_PAGINA_A_MEMORIA:
                trasladar_pagina_a_memoria(instruccion_pointer->pid, instruccion_pointer->numero_pagina, instruccion_pointer->numero_marco);
                break;
            case TRASLADAR_PROCESO_A_DISCO:
                trasladar_proceso_a_disco(instruccion_pointer->tabla_primer_nivel_pointer);
                break;
            case BORRAR_ARCHIVO_SWAP:
                borrar_archivo_swap(instruccion_pointer->pid);
                break;
            default:
                log_error(logger, "No existe el codigo de instruccion");
                break;
        }

        //indico a memoria que termine de ejecutar una instruccion swap
        if (instruccion_pointer->semaforo_pointer != NULL){
            sem_post(instruccion_pointer->semaforo_pointer);
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
    
    sprintf(filename, "%s/%d.swap", PATH_SWAP, pid);

    if ((archivo_swap = fopen(filename, "w")) == NULL){
        log_error(logger, "Error en 'crear_archivo_swap()': no se pudo crear el archivo swap");
        return;
    }

    escribir_ceros_archivo(archivo_swap, tamanio_proceso);

    fclose(archivo_swap);

    return;
}

void escribir_ceros_archivo(FILE * archivo, int32_t cantidad_ceros){
    char * ceros = malloc(cantidad_ceros);
    memset(ceros, 0, cantidad_ceros);
    fwrite(ceros, 1, cantidad_ceros, archivo);
    free(ceros);
}

void trasladar_pagina_a_disco(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    //TODO: CHECKEAR/TESTEAR

    /*  
        FIJATE QUE CAMBIE LOS PARAMETROS QUE RECIBE LA FUNCION
        HAY QUE PASAR LO QUE ESTA EN EL MARCO A LA PAGINA DEL ARCHIVO SWAP
        NO HAY QUE CAMBIAR NINGUN BIT DE PRESENCIA NI NADA
        BY: LEAN
    */
   /*
    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado
    void marco_temp[TAM_PAGINA]; // Marco temporal
    memcpy(marco_temp, marco, TAM_PAGINA); // Guardo el contenido del marco en el marco temporal

    FILE f;
    char swap_file[MAX_STRING_SIZE];
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    f = fopen(swap_file, "r+");

    fseek(f, TAM_PAGINA * numero_pagina, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fwrite(marco_temp, TAM_PAGINA, 1, f);

    fclose(f);*/

    return;
}

void trasladar_pagina_a_memoria(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    //TODO: CHECKEAR/TESTEAR
    
    /*  
        FIJATE QUE CAMBIE LOS PARAMETROS QUE RECIBE LA FUNCION
        EL NUMERO DE MARCO YA VIENE COMO PARAMETRO (la eleccion del marco se hace en alguna funcion en acciones.c)
        NO HAY QUE CAMBIAR NINGUN BIT DE PRESENCIA NI NADA
        BY: LEAN
    */
   /*
    int pag_de_proceso = numero_pagina;
    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado

    FILE f;
    char swap_file[MAX_STRING_SIZE]; 
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    f = fopen(swap_file, "r+");

    fseek(f, TAM_PAGINA * numero_pagina, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fread(marco, TAM_PAGINA, 1, f);
    
    fclose(f);*/

    return;
}

void trasladar_proceso_a_disco(tabla_primer_nivel* tabla_pointer){
    /*
        Itero en todas las paginas de la tabla buscando las que tengan presencia
        y las mando a disco.
        IMPORTANTE: CAMBIA EL VALOR DE PRESENCIA A FALSE DE LAS PAGINAS CON PRESENCIA
    */

    int i = 0, j = 0;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* pagina_actual_pointer;

    for (i = 0; i < tabla_pointer->cantidad_entradas; i++){

        tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, i);

        for( j = 0; j < tabla_segundo_nivel_pointer->cantidad_entradas; j++){
            pagina_actual_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, j);

            // si la pagina forma parte del conjunto residente la mando a disco
            if (pagina_actual_pointer->presencia == true){
                trasladar_pagina_a_disco(tabla_pointer->pid, i * ENTRADAS_POR_TABLA + j, pagina_actual_pointer->numero_marco);
                pagina_actual_pointer->presencia = false;
            }
        }

    }

    return;
}

void borrar_archivo_swap(int32_t pid){

    char swap_file[MAX_STRING_SIZE];

    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);

    if(remove(swap_file) == 0){
        log_debug(logger, "ELIMINAR ARCHIVO SWAP (PID = %d)", pid);
    } else {
        log_error(logger, "error al eliminar el archivo swap: %s",swap_file);
    }

}
