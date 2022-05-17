#include "kernel.h"

//TODO: implementar semaforos en todos los planificadores que utilicen variables globales
//TODO: implementar semaforos, si hace falta, cuando se llama a un planificador que sea llamado por varias funciones

bool es_algoritmo_srt(void){
    return strcmp(ALGORITMO_PLANIFICACION, "STR") == 0;
}

void ingresar_proceso_a_new(int signal){
    /*
        Escucha la signal SIGUSR1 que se activa cuando llega un proceso a
        NEW o a READY_SUSPENDIDO o se baja el grado de multiprogramacion
    */
    if (grado_multiprogramacion_actual < GRADO_MULTIPROGRAMACION){

        if (!queue_is_empty(cola_ready_suspendido)){
            transicion_ready_suspendido_ready();
        }
        else{
            transicion_new_ready();
        }
    }

}

void transicion_consola_new(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la transicion CONSOLA->NEW
    */

    pcb_t *pcb_pointer = generar_pcb(lista_instrucciones, tamanio_proceso, socket);

    queue_push(cola_new, pcb_pointer);

    contador_pid++;

    log_info(logger, "Nuevo proceso en NEW (pid = %d)", pcb_pointer->pid);

    kill(getpid(), SIGUSR1);
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

    pcb_pointer = queue_peek(cola_new);

    inicializar_estructuras_memoria(pcb_pointer);

    list_add(lista_ready, queue_pop(cola_new));

    log_info(logger, "NEW->READY (PID=%d)", pcb_pointer->pid);

    grado_multiprogramacion_actual++;

    if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
    else if (en_ejecucion == NULL) transicion_ready_ejec();
    
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

    log_info(logger, "EJEC -> BLOQUEADO (PID = %d)", en_ejecucion->pid);

    en_ejecucion = NULL;

    pthread_create(&hilo_proceso_bloqueado, NULL, esperar_tiempo_bloqueo, &datos_para_hilo);

    //guardo el pthread en la cola de threads para que estos sean esperados y liberar sus recursos

    pthread_mutex_lock(&mutex_cola_threads);
    queue_push(cola_threads, &hilo_proceso_bloqueado);
    pthread_mutex_unlock(&mutex_cola_threads);

    sem_post(&semaforo_cola_threads);
}

void *esperar_tiempo_bloqueo(void * datos){
    /*
        Hilo que maneja un proceso cuando se va a lista_bloqueado.
        Espera el tiempo de bloqueo y , si esta mas tiempo del permitido, se va a suspendido
    */

    datos_tiempo_bloqueo * datos_tiempo_bloqueo_pointer = datos; //paso de void* a datos_tiempo_bloqueo*

    //espero el tiempo de bloqueo
    if (datos_tiempo_bloqueo_pointer->tiempo_bloqueo <= TIEMPO_MAXIMO_BLOQUEADO){

        usleep(datos_tiempo_bloqueo_pointer->tiempo_bloqueo);

        //lo mando a READY

        list_add(lista_ready, list_remove(lista_bloqueado, get_indice_pcb_pointer(lista_bloqueado, datos_tiempo_bloqueo_pointer->pcb_pointer)));

        log_info(logger, "BLOQUEADO -> READY (PID = %d)", datos_tiempo_bloqueo_pointer->pcb_pointer->pid);

        if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
        else if (en_ejecucion == NULL) transicion_ready_ejec(); 
    }
    else{ //espero el tiempo maximo de bloqueado y lo mando a suspendido a seguir esperando por I/O

        usleep(TIEMPO_MAXIMO_BLOQUEADO);

        //suspendo el proceso

        memoria_suspender_proceso(datos_tiempo_bloqueo_pointer->pcb_pointer);

        grado_multiprogramacion_actual--;

        kill(getpid(), SIGUSR1);

        list_add(lista_bloqueado_suspendido, list_remove(lista_bloqueado, get_indice_pcb_pointer(lista_bloqueado, datos_tiempo_bloqueo_pointer->pcb_pointer)));

        log_info(logger, "BLOQUEADO -> BLOQUEADO_SUSPENDIDO (PID = %d)", datos_tiempo_bloqueo_pointer->pcb_pointer->pid);

        //espero el tiempo restante
        usleep(datos_tiempo_bloqueo_pointer->tiempo_bloqueo - TIEMPO_MAXIMO_BLOQUEADO);

        //ahora lo mando a READY_SUSPENDIDO

        queue_push(cola_ready_suspendido, list_remove(lista_bloqueado_suspendido, get_indice_pcb_pointer(lista_bloqueado_suspendido, datos_tiempo_bloqueo_pointer->pcb_pointer)));

        log_info(logger, "BLOQUEADO_SUSPENDIDO -> READY_SUSPENDIDO (PID = %d)", datos_tiempo_bloqueo_pointer->pcb_pointer->pid);
        
        kill(getpid(), SIGUSR1);
    }

    return NULL;
}

void transicion_ready_suspendido_ready(void){

    pcb_t* pcb_pointer = queue_pop(cola_ready_suspendido);

    memoria_volver_de_suspendido(pcb_pointer);

    list_add(lista_ready, pcb_pointer);

    grado_multiprogramacion_actual++;

    if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
    else if (en_ejecucion == NULL) transicion_ready_ejec();         

}

int get_indice_pcb_pointer(t_list* lista, pcb_t* pcb_pointer){

    int i = 0;

    if (list_is_empty(lista)) return -1;

    for (i = 0; i < list_size(lista); i++){

        if (list_get(lista, i) == pcb_pointer) return i;

    }

    return -1;
}

void *hacer_join_hilos_mediano_plazo(void *arg){
    /*
        Hilo que espera a que terminen los hilos de funcion esperar_tiempo_bloqueo()
    */

    pthread_t *thread_pointer = NULL;

    while(1){

        sem_wait(&semaforo_cola_threads);

        pthread_mutex_lock(&mutex_cola_threads);
        thread_pointer = queue_pop(cola_threads);
        pthread_mutex_unlock(&mutex_cola_threads);
    
        pthread_join(*thread_pointer, NULL);
    }

    return NULL;
}