#include "kernel.h"

//TODO: implementar semaforos en todos los planificadores que utilicen variables globales
//TODO: implementar semaforos, si hace falta, cuando se llama a un planificador que sea llamado por varias funciones

bool es_algoritmo_srt(void){
    return strcmp(ALGORITMO_PLANIFICACION, "STR") == 0;
}

void transicion_consola_new(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la transicion CONSOLA->NEW
    */

    pcb_t *pcb_pointer = generar_pcb(lista_instrucciones, tamanio_proceso, socket);

    queue_push(cola_new, pcb_pointer);

    contador_pid++;

    log_info(logger, "Nuevo proceso en NEW (pid = %d)", pcb_pointer->pid);

    transicion_new_ready();
}

void transicion_ejec_ready(void){
    /*
        Gestiona la transicion EJEC->READY
    */

    if (en_ejecucion == NULL){
        log_error(logger, "error en transicion_ejec_ready(): 'en_ejecucion' es NULL");
        return;
    }
    
    if (es_algoritmo_srt()) sumar_duracion_rafaga(en_ejecucion);

    list_add(lista_ready, en_ejecucion);

    log_info(logger, "EJEC->READY (pid = %d)", en_ejecucion->pid);

    en_ejecucion = NULL;

    transicion_ready_ejec();
}

int seleccionar_proceso_menor_estimacion(void){
    /*
        Recorre la lista de ready comparando las estimaciones de rafagas
        y devuelve la posicion en la lista de ready del pcb del proceso con menor estimacion.
    */

    pcb_t * pcb_actual = NULL;
    pcb_t * pcb_menor_rafaga = NULL;
    int indice_menor_rafaga = 0;
    int i = 0;

    if (!list_is_empty(lista_ready)){
        pcb_menor_rafaga = list_get(lista_ready, 0);
    }

    for (i = 1; i < list_size(lista_ready); i++){

        pcb_actual = list_get(lista_ready, i);

        if (pcb_actual->estimacion_rafaga < pcb_menor_rafaga->estimacion_rafaga){
            pcb_menor_rafaga = pcb_actual;
            indice_menor_rafaga = i;
        }

    }

    return indice_menor_rafaga;
}

void transicion_ready_ejec(void){
    /*
        Gestiona la transicion READY->EJEC
    */

    if (en_ejecucion != NULL){
        log_error(logger, "error en transicion_ready_ejec(): 'en_ejecucion' no es NULL");
        return;
    }

    if (list_size(lista_ready) == 0){
        log_info(logger, "No hay procesos en READY para llevar a EJEC");
        return;
    }

    if (es_algoritmo_srt()){
        en_ejecucion = list_remove(lista_ready, seleccionar_proceso_menor_estimacion());
    }
    else{//si es fifo
        en_ejecucion = list_remove(lista_ready, 0);
    } 

    if (en_ejecucion->tabla_paginas == -1){
        log_error(logger, "error en transicion_ready_ejec(): el proceso no posee una tabla de paginas reservada");
        return;
    }

    if (es_algoritmo_srt()) actualizar_timestamp(en_ejecucion);

    enviar_pcb_cpu(en_ejecucion);

    log_info(logger, "READY->EJEC (PID=%d)", en_ejecucion->pid);
}

void transicion_new_ready(void){
    /*
        Gestiona la transicion NEW->READY
    */
    
    pcb_t *pcb_pointer;

    if (grado_multiprogramacion_actual < GRADO_MULTIPROGRAMACION){

        pcb_pointer = queue_peek(cola_new);

        inicializar_estructuras_memoria(pcb_pointer);

        list_add(lista_ready, queue_pop(cola_new));

        log_info(logger, "NEW->READY (PID=%d)", pcb_pointer->pid);

        grado_multiprogramacion_actual++;

        if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
        else if (en_ejecucion == NULL) transicion_ready_ejec();
    }
}

void transicion_ejec_exit(void){
    /*
        Gestiona la transicion EJEC->EXIT
    */

    if (en_ejecucion == NULL){
        log_error(logger, "error en transicion_ejec_exit(): 'en_ejecucion' es NULL");
        return;
    }

    liberar_estructuras_memoria();

    log_info(logger, "EJEC->EXIT (PID=%d)", en_ejecucion->pid);

    finalizar_conexion_consola(en_ejecucion);

    liberar_memoria_pcb(en_ejecucion);
        
    en_ejecucion = NULL;

    grado_multiprogramacion_actual--;
}

void transicion_ejec_bloqueado(int32_t tiempo_bloqueo){
    /*
        Gestiona la transicion EJEC->BLOQUEADO
    */

    pthread_t hilo_proceso_bloqueado;
    datos_tiempo_bloqueo datos_para_hilo = {tiempo_bloqueo, en_ejecucion};

    list_add(lista_bloqueado, en_ejecucion);

    en_ejecucion = NULL;

    pthread_create(&hilo_proceso_bloqueado, NULL, esperar_tiempo_bloqueo, &datos_para_hilo);
}

void *esperar_tiempo_bloqueo(void * datos){
    /*
        Hilo que maneja un proceso cuando se va a lista_bloqueado
    */

    datos_tiempo_bloqueo * datos_tiempo_bloqueo_pointer = datos; //paso de void* a datos_tiempo_bloqueo*

    usleep(datos_tiempo_bloqueo_pointer->tiempo_bloqueo);

    list_remove(lista_bloqueado, get_indice_pcb_pointer(lista_bloqueado, datos_tiempo_bloqueo_pointer->pcb_pointer));

    if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
    else if (en_ejecucion == NULL) transicion_ready_ejec();  

    return NULL;
}

int get_indice_pcb_pointer(t_list* lista, pcb_t* pcb_pointer){

    int i = 0;

    if (list_is_empty(lista)) return -1;

    for (i = 0; i < list_size(lista); i++){

        if (list_get(lista, i) == pcb_pointer) return i;

    }

    return -1;
}