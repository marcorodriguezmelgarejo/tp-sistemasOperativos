#include "kernel.h"

//TODO: implementar semaforos en todos los planificadores que utilicen variables globales
//TODO: implementar semaforos, si hace falta, cuando se llama a un planificador que sea llamado por varias funciones

bool es_algoritmo_srt(void){
    return strcmp(ALGORITMO_PLANIFICACION, "STR") == 0;
}

void sumar_duracion_rafaga(pcb_t * pcb_pointer){
    pcb_pointer->duracion_real_ultima_rafaga += get_tiempo_transcurrido(pcb_pointer->timestamp);
}

void transicion_ejec_ready(void){
    /*
        Gestiona la transicion EJEC->READY
    */
    
    if (es_algoritmo_srt()){
        sumar_duracion_rafaga(en_ejecucion);
    }

    list_add(lista_ready, &en_ejecucion);

    log_info(logger, "EJEC->READY (pid = %d)", en_ejecucion->pid);

    en_ejecucion = NULL;

    transicion_ready_ejec();
}

pcb_t* seleccionar_proceso_menor_estimacion(void){
    /*
        Recorre la lista de ready comparando las estimaciones de rafagas
        y devuelve un puntero al pcb del proceso con menor estimacion
    */

    pcb_t ** pcb_actual = NULL;
    pcb_t ** pcb_menor_rafaga = NULL;

    t_list_iterator* iterator_ready = list_iterator_create(lista_ready);

    if (list_iterator_has_next(iterator_ready)){
        pcb_menor_rafaga = list_iterator_next(iterator_ready);
    }

	while(list_iterator_has_next(iterator_ready)){

        pcb_actual = list_iterator_next(iterator_ready);
        if ((*pcb_actual)->estimacion_rafaga < (*pcb_menor_rafaga)->estimacion_rafaga) pcb_menor_rafaga = pcb_actual;
    }

    list_iterator_destroy(iterator_ready);

    return *pcb_menor_rafaga;
}

void transicion_ready_ejec(void){
    /*
        Gestiona la transicion READY->EJEC
    */

    pcb_t** pcb_a_ejecutar = NULL;

    if (en_ejecucion != NULL){
        log_error(logger, "error en planificador_corto_plazo_ejec(): 'en_ejecucion' no es NULL");
        return;
    }

    if (list_size(lista_ready) == 0){
        log_info(logger, "No hay procesos en READY para llevar a EJEC");
        return;
    }

    if (es_algoritmo_srt()){

        en_ejecucion = seleccionar_proceso_menor_estimacion();

    }
    else{ //si es fifo

        pcb_a_ejecutar = list_get(lista_ready, 0);
        en_ejecucion = *pcb_a_ejecutar;
    }

    if (en_ejecucion->tabla_paginas == -1){
        log_error(logger, "El proceso a punto de ejecutarse no posee una tabla de paginas reservada");
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
    
    pcb_t **pcb_pointer;

    if (grado_multiprogramacion_actual < GRADO_MULTIPROGRAMACION){

        pcb_pointer = queue_peek(cola_new);
        inicializar_estructuras_memoria();
        list_add(lista_ready, queue_pop(cola_new));

        log_info(logger, "NEW->READY (PID=%d)", (*pcb_pointer)->pid);

        grado_multiprogramacion_actual++;

        enviar_interrupcion_cpu();
    }
}

void transicion_ejec_exit(void){
    /*
        Gestiona la transicion EJEC->EXIT
    */

    liberar_estructuras_memoria();

    finalizar_conexion_consola(en_ejecucion->pid);

    log_info(logger, "EJEC->EXIT (PID=%d)", en_ejecucion->pid);
        
    en_ejecucion = NULL;

    grado_multiprogramacion_actual--;
}
