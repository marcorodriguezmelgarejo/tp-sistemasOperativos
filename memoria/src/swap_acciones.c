#include "memoria.h"

void acciones_trasladar_pagina_a_disco(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco){

    enviar_instruccion_swap_TRASLADAR_PAGINA_A_DISCO(tabla_pointer->pid, numero_pagina, numero_pagina);
    
    quitar_pagina_lista_paginas_cargadas(tabla_pointer, numero_pagina);

    marcar_marco_como_libre(numero_marco);
    
    tabla_pointer->tamanio_conjunto_residente -= 1;

    return;
}

void acciones_trasladar_pagina_a_memoria(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco){

    enviar_instruccion_swap_TRASLADAR_PAGINA_A_MEMORIA(tabla_pointer->pid, numero_pagina, numero_marco);

    agregar_pagina_lista_paginas_cargadas(tabla_pointer, numero_pagina);

    marcar_marco_como_ocupado(numero_marco);
    
    tabla_pointer->tamanio_conjunto_residente += 1;

    return;
}

void enviar_instruccion_swap_CREAR_ARCHIVO_SWAP(int32_t pid, int32_t tamanio_proceso){
    
    instruccion_swap instruccion;
    sem_t semaforo;
    
    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = CREAR_ARCHIVO_SWAP;
    instruccion.pid = pid;
    instruccion.tamanio_proceso = tamanio_proceso;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);
    
    sem_wait(&semaforo); //espera a que termine la instruccion swap
    
    sem_destroy(&semaforo);
}

void enviar_instruccion_swap_TRASLADAR_PROCESO_A_DISCO(tabla_primer_nivel* tabla_pointer){

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PROCESO_A_DISCO;
    instruccion.tabla_primer_nivel_pointer = tabla_pointer;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);
}

void enviar_instruccion_swap_BORRAR_ARCHIVO_SWAP(int32_t pid){

    instruccion_swap instruccion;
    sem_t semaforo;
    
    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = BORRAR_ARCHIVO_SWAP;
    instruccion.pid = pid;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);
}

void enviar_instruccion_swap_TRASLADAR_PAGINA_A_DISCO(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PAGINA_A_DISCO;
    instruccion.pid = pid;
    instruccion.numero_pagina = numero_pagina;
    instruccion.numero_marco = numero_marco;
    instruccion.semaforo_pointer = &semaforo;

    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo);

    sem_destroy(&semaforo);
}

void enviar_instruccion_swap_TRASLADAR_PAGINA_A_MEMORIA(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PAGINA_A_MEMORIA;
    instruccion.pid = pid;
    instruccion.numero_pagina = numero_pagina;
    instruccion.numero_marco = numero_marco;
    instruccion.semaforo_pointer = &semaforo;

    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo);

    sem_destroy(&semaforo);
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