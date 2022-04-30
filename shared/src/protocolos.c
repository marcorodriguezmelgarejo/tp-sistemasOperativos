#include "protocolos.h"

bool sockets_enviar_pcb(int socket, pcb_t pcb, t_log* logger){

    char *instruccion_buffer;
    char delimiter[] = "\n";
    void * data_pointer;
    size_t tamanio  = 0;

    // Envio primero la lista de instrucciones

    instruccion_buffer = strtok(pcb.lista_instrucciones, delimiter);
    
    //voy parseando las diferentes instrucciones
    while( instruccion_buffer != NULL ) {
        if (sockets_enviar_string(socket, instruccion_buffer, logger) == false) return false;
        instruccion_buffer = strtok(NULL, delimiter);
    }

    if (sockets_enviar_string(socket, "FIN", logger) == false) return false;

    // Envio el resto de la pcb

    //guardo espacio para 5 variables de 4 bytes (int32_t)
    if ((data_pointer = malloc(TAMANIO_PCB_SERIALIZADA)) == NULL){
        log_error(logger, "Error al hacer malloc en sockets_enviar_pcb()");
        return false;
    }

    memcpy(data_pointer, &(pcb.pid), sizeof pcb.pid);
    tamanio += sizeof pcb.pid;
    memcpy(data_pointer + tamanio, &(pcb.tamanio), sizeof pcb.tamanio);
    tamanio += sizeof pcb.tamanio;
    memcpy(data_pointer + tamanio, &(pcb.program_counter), sizeof pcb.program_counter);
    tamanio += sizeof pcb.program_counter;
    memcpy(data_pointer + tamanio, &(pcb.tabla_paginas), sizeof pcb.tabla_paginas);
    tamanio += sizeof pcb.tabla_paginas;
    memcpy(data_pointer + tamanio, &(pcb.estimacion_rafaga), sizeof pcb.estimacion_rafaga);
    tamanio += sizeof pcb.estimacion_rafaga;

    if (sockets_enviar_dato(socket, data_pointer, tamanio, logger) == false){
        free(data_pointer);
        return false;
    }

    free(data_pointer);
    return true;
}

bool sockets_recibir_pcb(int socket, pcb_t* pcb_pointer, t_log* logger){

    char instruccion_buffer[MAX_INSTRUCCION_SIZE] = "";
    void *buffer;
    size_t tamanio = 0;

    // Primero recibo la lista de instrucciones

    while(strcmp(instruccion_buffer, "FIN") != 0){
        sockets_recibir_string(socket, instruccion_buffer, logger);

        if (strcmp(instruccion_buffer, "FIN") == 0) break;

        //TODO: RESERVAR MEMORIA PARA LISTA_INSTRUCCIONES
        strcat(instruccion_buffer, "\n"); //le agrego el \n
        strcat(pcb_pointer->lista_instrucciones, instruccion_buffer);
    }

    // Recibo el resto de la pcb

    if ((buffer = malloc(TAMANIO_PCB_SERIALIZADA)) == NULL){
        log_error(logger, "Error al hacer malloc en sockets_recibir_pcb()");
        return false;
    }

    if (sockets_recibir_dato(socket, buffer, TAMANIO_PCB_SERIALIZADA, logger) == false){
        free(buffer);
        return false;
    }

    memcpy(&(pcb_pointer->pid), buffer, sizeof pcb_pointer->pid);
    tamanio += sizeof pcb_pointer->pid;
    memcpy(&(pcb_pointer->tamanio), buffer + tamanio, sizeof pcb_pointer->tamanio);
    tamanio += sizeof pcb_pointer->tamanio;
    memcpy(&(pcb_pointer->program_counter), buffer + tamanio, sizeof pcb_pointer->program_counter);
    tamanio += sizeof pcb_pointer->program_counter;
    memcpy(&(pcb_pointer->tabla_paginas), buffer + tamanio, sizeof pcb_pointer->tabla_paginas);
    tamanio += sizeof pcb_pointer->tabla_paginas;
    memcpy(&(pcb_pointer->estimacion_rafaga), buffer + tamanio, sizeof pcb_pointer->estimacion_rafaga);

    free(buffer);
    return true;
}