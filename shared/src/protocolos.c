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

    //guardo espacio para todas las variables de la pcb menos la lista de instrucciones
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
    memcpy(data_pointer + tamanio, &(pcb.duracion_real_ultima_rafaga), sizeof pcb.duracion_real_ultima_rafaga);
    tamanio += sizeof pcb.duracion_real_ultima_rafaga;
    memcpy(data_pointer + tamanio, &(pcb.timestamp), sizeof pcb.timestamp);
    tamanio += sizeof pcb.timestamp;
    memcpy(data_pointer + tamanio, &(pcb.consola_socket), sizeof pcb.consola_socket);
    tamanio += sizeof pcb.consola_socket;

    if (sockets_enviar_dato(socket, data_pointer, tamanio, logger) == false){
        free(data_pointer);
        return false;
    }

    free(data_pointer);
    return true;
}

bool sockets_recibir_pcb(int socket, pcb_t* pcb_pointer, t_log* logger){
    /*
        ACLARACION: pcb_pointer->lista_instrucciones no debe tener memoria ya reservada,
        se reserva en esta funcion. Luego habra que hacerle un free().
        pcb_pointer->lista_instrucciones debera ser NULL
    */

    char instruccion_buffer[MAX_INSTRUCCION_SIZE] = "";
    void *buffer;
    size_t tamanio = 0;

    if (pcb_pointer->lista_instrucciones != NULL){
        log_error(logger, "error en sockets_recibir_pcb(): pcb_pointer debe ser NULL\n");
        return false;
    }

    // Primero recibo la lista de instrucciones

    while(strcmp(instruccion_buffer, "FIN") != 0){
        sockets_recibir_string(socket, instruccion_buffer, logger);

        if (strcmp(instruccion_buffer, "FIN") == 0) break;

        //si todavia no tiene memoria alocada
        if (pcb_pointer->lista_instrucciones == NULL){
            if ((pcb_pointer->lista_instrucciones = malloc(strlen(instruccion_buffer) + 1)) == NULL){
                log_error(logger, "error en sockets_recibir_pcb(): error al hacer malloc()");
                return false;
            }
            strcpy(pcb_pointer->lista_instrucciones, instruccion_buffer);
        }
        else{
            if ((pcb_pointer->lista_instrucciones = realloc(pcb_pointer->lista_instrucciones, strlen(pcb_pointer->lista_instrucciones) + strlen(instruccion_buffer) + 2)) == NULL){
                log_error(logger, "error en sockets_recibir_pcb(): error al hacer realloc()");
                return false;
            }
            strcat(pcb_pointer->lista_instrucciones, "\n"); //le agrego el \n
            strcat(pcb_pointer->lista_instrucciones, instruccion_buffer);
        }
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
    tamanio += sizeof pcb_pointer->estimacion_rafaga;
    memcpy(&(pcb_pointer->duracion_real_ultima_rafaga), buffer + tamanio, sizeof pcb_pointer->duracion_real_ultima_rafaga);
    tamanio += sizeof pcb_pointer->duracion_real_ultima_rafaga;
    memcpy(&(pcb_pointer->timestamp), buffer + tamanio, sizeof pcb_pointer->timestamp);
    tamanio += sizeof pcb_pointer->timestamp;
    memcpy(&(pcb_pointer->consola_socket), buffer + tamanio, sizeof pcb_pointer->consola_socket);

    free(buffer);
    return true;
}