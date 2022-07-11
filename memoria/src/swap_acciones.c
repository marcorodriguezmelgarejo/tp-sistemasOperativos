#include "memoria.h"

void acciones_trasladar_pagina_a_memoria(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco){

    enviar_instruccion_swap_TRASLADAR_PAGINA_A_MEMORIA(tabla_pointer->pid, numero_pagina, numero_marco);

    agregar_pagina_lista_paginas_cargadas(tabla_pointer, numero_pagina);

    marcar_marco_como_ocupado(numero_marco);
    
    tabla_pointer->tamanio_conjunto_residente += 1;

    return;
}

void enviar_instruccion_swap_TRASLADAR_PAGINA_A_MEMORIA(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    trasladar_pagina_a_memoria(pid, numero_pagina, numero_marco);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);
}

void enviar_instruccion_swap_CREAR_ARCHIVO_SWAP(int32_t pid, int32_t tamanio_proceso){
    
    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    crear_archivo_swap(pid, tamanio_proceso);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);
}

void enviar_instruccion_swap_TRASLADAR_PROCESO_A_DISCO(tabla_primer_nivel* tabla_pointer){

    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    trasladar_proceso_a_disco(tabla_pointer);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);
}

void enviar_instruccion_swap_BORRAR_ARCHIVO_SWAP(int32_t pid){

    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    borrar_archivo_swap(pid);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);
}

void enviar_instruccion_swap_TRASLADAR_PAGINA_A_DISCO(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    trasladar_pagina_a_disco(pid, numero_pagina, numero_marco, true);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);
}