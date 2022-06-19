#include "memoria.h"

tabla_primer_nivel* inicializar_proceso(int32_t pid, int32_t tamanio_proceso){

    /*
        Inicializa el archivo swap y la tabla de paginas para el proceso
    */

    if (tamanio_proceso >= TAM_PAGINA * ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA){
        log_error(logger, "El tamanio de proceso excede la cantidad de paginas asignables");
    }

    tabla_primer_nivel* tabla_primer_nivel_pointer;
    instruccion_swap instruccion;
    sem_t semaforo;

    int32_t cantidad_paginas = ceil(tamanio_proceso / TAM_PAGINA); //de 1 a ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_primer_nivel = ceil(cantidad_paginas / ENTRADAS_POR_TABLA); //de 1 a ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_segundo_nivel_ultima_entrada = cantidad_paginas % ENTRADAS_POR_TABLA; //de 1 a ENTRADAS_POR_TABLA

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = CREAR_ARCHIVO_SWAP;
    instruccion.pid = pid;
    instruccion.tamanio_proceso = tamanio_proceso;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine la instruccion swap

    sem_destroy(&semaforo);
    
    tabla_primer_nivel_pointer = crear_tabla_paginas_proceso(pid, cantidad_entradas_primer_nivel, cantidad_entradas_segundo_nivel_ultima_entrada);

    return tabla_primer_nivel_pointer;
}

tabla_primer_nivel* crear_tabla_paginas_proceso(int32_t pid, int32_t cantidad_entradas_primer_nivel, int32_t cantidad_entradas_segundo_nivel_ultima_entrada){
    /*
        Reserva memoria para 1 tabla de paginas de primer nivel y para las tablas de segundo nivel necesarias.
    */

    int i = 0, j = 0, entradas_a_crear = 0;
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    if ((tabla_primer_nivel_pointer = malloc(sizeof *tabla_primer_nivel_pointer)) == NULL){
        log_error(logger, "error al hacer malloc");
        return NULL;
    }

    (*tabla_primer_nivel_pointer).pid = pid;
    (*tabla_primer_nivel_pointer).cantidad_entradas = cantidad_entradas_primer_nivel;
    (*tabla_primer_nivel_pointer).lista_de_tabla_segundo_nivel = list_create();

    //reservo memoria e inicializo las tablas de segundo nivel
    for (i = 0; i > cantidad_entradas_primer_nivel; i++){

        if ((tabla_segundo_nivel_pointer = malloc(sizeof *tabla_segundo_nivel_pointer)) == NULL){
            log_error(logger, "error al hacer malloc");
            return NULL;
        }

        //si no es la ultima tabla de segundo nivel  
        if (i != cantidad_entradas_primer_nivel - 1){
            entradas_a_crear = ENTRADAS_POR_TABLA;
        }
        else{
            entradas_a_crear = cantidad_entradas_segundo_nivel_ultima_entrada;
        }

        (*tabla_segundo_nivel_pointer).cantidad_entradas = entradas_a_crear;
        (*tabla_segundo_nivel_pointer).lista_de_entradas = list_create();

        //reservo memoria e inicializo las entradas de segundo nivel
        for (j = 0; j < entradas_a_crear; j++){

            if ((entrada_segundo_nivel_pointer = malloc(sizeof *entrada_segundo_nivel_pointer)) == NULL){
                log_error(logger, "error al hacer malloc");
                return NULL;
            }

            (*entrada_segundo_nivel_pointer).numero_marco = -1;
            (*entrada_segundo_nivel_pointer).presencia = false;
            (*entrada_segundo_nivel_pointer).usado = false;
            (*entrada_segundo_nivel_pointer).modificado = false;

            list_add((*tabla_segundo_nivel_pointer).lista_de_entradas, entrada_segundo_nivel_pointer);
        }

    }

    //agrego el puntero de la tabla de primer nivel a la lista global

    list_add(lista_tabla_primer_nivel, tabla_primer_nivel_pointer);

    return tabla_primer_nivel_pointer;
}

void liberar_memoria_tabla_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Hace todos los frees correspondientes para liberar memoria alocada para una tabla de primer nivel
    */

    int i = 0, index = 0;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;

    for (i = 0; i < tabla_pointer->cantidad_entradas; i++){
        tabla_segundo_nivel_pointer = list_remove(tabla_pointer->lista_de_tabla_segundo_nivel, 0);
        list_destroy_and_destroy_elements(tabla_segundo_nivel_pointer->lista_de_entradas, free);
    }

    list_destroy(tabla_pointer->lista_de_tabla_segundo_nivel);

    if ((index = get_indice_tabla_pointer(lista_tabla_primer_nivel, tabla_pointer)) == -1){
        log_error(logger, "la tabla a liberar no se encuentra en la lista");
        return;
    }

    list_remove(lista_tabla_primer_nivel, index);
}


void suspender_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Envia instruccion a swap para mandar todo el conjunto residente del proceso a disco
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PROCESO_A_DISCO;
    instruccion.tabla_primer_nivel_pointer = tabla_pointer;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);

    return;
}

void finalizar_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Borra el archivo de swap del proceso y libera la tabla de primer nivel del proceso
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    // Borramos el archivo de swap del proceso
    instruccion.numero_instruccion = BORRAR_ARCHIVO_SWAP;
    instruccion.pid = tabla_pointer->pid;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);

    // Liberamos las paginas de memoria principal
    liberar_memoria_tabla_proceso(tabla_pointer);

    return;
}

int32_t acceder_tabla_primer_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){
    //TODO: IMPLEMENTAR

    // SE DEVUELVE EL INDICE DE LA LISTA tabla_pointer->lista_de_entradas EN DONDE SE ENCUENTRE LA PAGINA
    
    return 0;
}

int32_t acceder_tabla_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t pagina){
    //TODO: IMPLEMENTAR

    // HAY QUE TRAER LA PAGINA A MEMORIA SI NO LO ESTA (Y ELEGIR UN MARCO A REEMPLAZAR SI NO HAY MAS
    // ESPACIO EN EL CONJUNTO RESIDENTE DEL PROCESO)

    // PARA EL ALGORITMO DE REEMPLAZO CAPAZ HAY QUE AGREGAR UN NUMERO DE PAGINA EN LA TABLA DE PRIMER NIVEL
    // QUE SERIA DONDE APUNTA EL CLOCK.

    return -1;
}

int32_t acceder_espacio_usuario_lectura(int32_t numero_marco, int32_t desplazamiento){
    //TODO: IMPLEMENTAR

    // LA PAGINA SIEMPRE ESTA EN MEMORIA
    return 0;
}

bool acceder_espacio_usuario_escritura(int32_t numero_marco, int32_t desplazamiento, int32_t valor){
    //TODO: IMPLEMENTAR

    // LA PAGINA SIEMPRE ESTA EN MEMORIA
    return false;
}

int get_indice_tabla_pointer(t_list* lista, tabla_primer_nivel* tabla_pointer){

    // Devuelve -1 si no se encuentra en la lista

    int i = 0;

    if (list_is_empty(lista)) return -1;

    for (i = 0; i < list_size(lista); i++){

        if (list_get(lista, i) == tabla_pointer) return i;

    }

    return -1;
}

void enviar_instruccion_swap(instruccion_swap instruccion){
    /*
        Reserva memoria para una instruccion y la pushea
        en la cola utilizando los semaforos correspondientes.
    */

    instruccion_swap* instruccion_pointer;

    if ((instruccion_pointer = malloc(sizeof instruccion)) == NULL){
        log_error(logger, "error al hacer malloc en enviar_instruccion_swap()");
        return;
    }
    
    *instruccion_pointer = instruccion;

    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    queue_push(cola_instrucciones_swap, instruccion_pointer);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);

    sem_post(&contador_cola_instrucciones_swap);

}