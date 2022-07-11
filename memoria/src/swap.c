#include "memoria.h"

void crear_archivo_swap(int32_t pid, int32_t tamanio_proceso){
    /*
        Crea un archivo con el nombre <pid>.swap y escribe ceros con cantidad 'tamanio_proceso'
    */

    char filename[MAX_STRING_SIZE];
    FILE * archivo_swap;
    
    usleep(milisegundos_a_microsegundos(RETARDO_SWAP));

    sprintf(filename, "%s/%d.swap", PATH_SWAP, pid);

    if ((archivo_swap = fopen(filename, "w")) == NULL){
        log_error(logger, "Error en 'crear_archivo_swap()': no se pudo crear el archivo swap");
        return;
    }

    escribir_ceros_archivo(archivo_swap, tamanio_proceso);

    fclose(archivo_swap);

    log_debug(logger, "SWAP: CREAR ARCHIVO (PID = %d)", pid);

    return;
}

void escribir_ceros_archivo(FILE * archivo, int32_t cantidad_ceros){
    char * ceros = malloc(cantidad_ceros);
    memset(ceros, 0, cantidad_ceros);
    fwrite(ceros, 1, cantidad_ceros, archivo);
    free(ceros);
}

void trasladar_pagina_a_disco(int32_t pid, int32_t numero_pagina, int32_t numero_marco, bool esperar){

    if (esperar == true){
        usleep(milisegundos_a_microsegundos(RETARDO_SWAP));
    }

    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado
    void * marco_temp = malloc(TAM_PAGINA); // Marco temporal
    memcpy(marco_temp, marco, TAM_PAGINA); // Guardo el contenido del marco en el marco temporal

    FILE * archivo;
    char swap_file[MAX_STRING_SIZE];
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    archivo = fopen(swap_file, "r+");

    fseek(archivo, TAM_PAGINA * numero_pagina, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fwrite(marco_temp, TAM_PAGINA, 1, archivo);

    free(marco_temp);
    fclose(archivo);

    log_debug(logger, "SWAP: TRASLADAR PAGINA A DISCO (PID = %d, numero de pagina = %d, numero de marco = %d)", pid, numero_pagina, numero_marco);

    return;
}

void trasladar_pagina_a_memoria(int32_t pid, int32_t numero_pagina, int32_t numero_marco){

    usleep(milisegundos_a_microsegundos(RETARDO_SWAP));
    
    void* marco = espacio_usuario + numero_marco * TAM_PAGINA; // Puntero al marco indicado

    FILE* archivo;
    char swap_file[MAX_STRING_SIZE]; 
    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);
    archivo = fopen(swap_file, "r");

    fseek(archivo, TAM_PAGINA * numero_pagina, SEEK_SET); //Si tenemos el marco y pid es simplemente buscar en la lista el match
    fread(marco, TAM_PAGINA, 1, archivo);
    
    fclose(archivo);

    log_debug(logger, "SWAP: TRASLADAR PAGINA A MEMORIA (PID = %d, numero de pagina = %d, numero de marco = %d)", pid, numero_pagina, numero_marco);

    return;
}

void trasladar_proceso_a_disco(tabla_primer_nivel* tabla_pointer){
    /*
        Itero en todas las paginas de la tabla buscando las que tengan presencia
        y las mando a disco.//TODO: CHECKEAR/TESTEAR
        IMPORTANTE: CAMBIA EL VALOR DE PRESENCIA A FALSE DE LAS PAGINAS CON PRESENCIA
    */

    usleep(milisegundos_a_microsegundos(RETARDO_SWAP));

    int i = 0, j = 0;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* pagina_actual_pointer;

    for (i = 0; i < tabla_pointer->cantidad_entradas; i++){

        tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, i);

        for( j = 0; j < tabla_segundo_nivel_pointer->cantidad_entradas; j++){
            pagina_actual_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, j);

            // si la pagina forma parte del conjunto residente la mando a disco
            if (pagina_actual_pointer->presencia == true){
                trasladar_pagina_a_disco(tabla_pointer->pid, i * ENTRADAS_POR_TABLA + j, pagina_actual_pointer->numero_marco, false);
                pagina_actual_pointer->presencia = false;
            }
        }

    }

    log_debug(logger, "SWAP: TRASLADAR PROCESO A DISCO (PID = %d)", tabla_pointer->pid);

    return;
}

void borrar_archivo_swap(int32_t pid){

    usleep(milisegundos_a_microsegundos(RETARDO_SWAP));

    char swap_file[MAX_STRING_SIZE];

    sprintf(swap_file, "%s/%d.swap", PATH_SWAP, pid);

    if(remove(swap_file) != 0){
        log_error(logger, "error al eliminar el archivo swap: %s",swap_file);
    }
    else{
        log_debug(logger, "SWAP: BORRAR ARCHIVO (PID = %d)", pid);
    }
}
