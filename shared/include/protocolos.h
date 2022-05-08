#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/queue.h> 
#include <commons/collections/list.h> 
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include "sockets.h"

#define OK_MESSAGE 0
#define ERROR_MESSAGE 1
#define DUMMY_VALUE 0 

#define MAX_INSTRUCCION_SIZE 50

//si se modifica la pcb se deben modificar las funciones que la envian y reciben
typedef struct pcb_t{
    int32_t pid;
    int32_t tamanio;
    int32_t program_counter;
    char * lista_instrucciones; //instrucciones separadas por un \n
    int32_t tabla_paginas;
    int32_t estimacion_rafaga;
    time_t timestamp; // (en milisegundos) para medir cuanto estuvo ejecutandose el proceso en CPU
} pcb_t;

// TODO: ENVIAR TAMBIEN LA TIMESTAMP

#define TAMANIO_PCB_SERIALIZADA 20 //sin contar la lista de instrucciones

// ACLARACION: todas las funciones devuelven 'true' en caso de exito. 'false' si hubo un error

bool sockets_enviar_pcb(int socket, pcb_t pcb, t_log* logger);
bool sockets_recibir_pcb(int socket, pcb_t *pcb_pointer, t_log* logger); //VER ACLARACION EN LA DEFINICION

#endif