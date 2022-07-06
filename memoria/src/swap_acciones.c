#include "memoria.h"

void acciones_trasladar_pagina_a_disco(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    /*
        Envia instruccion a swap para trasladar la pagina numero 'numero_pagina' del proceso asociado a 'tabla_pointer' a disco.
        La misma esta ubicada en el marco numero 'numero_marco'.
        Espera que la accion en swap se realize.
    */

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

    return;
}

void acciones_trasladar_pagina_a_memoria(int32_t pid, int32_t numero_pagina, int32_t numero_marco){

    /*
        Envia instruccion a swap para trasladar la pagina numero 'numero_pagina' del proceso asociado a 'tabla_pointer' a memoria
        en el marco numero 'numero_marco'.
        Espera que la accion en swap se realize.
    */

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

    return;
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