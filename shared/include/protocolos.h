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

// motivos de los mensajes que manda kernel a cpu
#define INTERRUPCION_CPU 0
#define FIN_CPU 1

// motivos de los mensajes que manda cpu a memoria
#define ACCESO_TABLA_PRIMER_NIVEL 0
#define ACCESO_TABLA_SEGUNDO_NIVEL 1
#define LECTURA_EN_ESPACIO_USUARIO 2
#define ESCRITURA_EN_ESPACIO_USUARIO 3

//si se modifica la pcb se deben modificar las funciones que la envian y reciben
typedef struct pcb_t{
    int32_t pid;
    int32_t tamanio;
    int32_t program_counter;
    char* lista_instrucciones; //instrucciones separadas por un \n
    void* tabla_paginas; // NO SE USA. La tabla de paginas se identifica por el pid del proceso.
    int32_t estimacion_rafaga;
    int32_t duracion_real_ultima_rafaga;
    uint64_t timestamp; // NO SE USA
    int32_t consola_socket; //el fd del socket mediante el cual el kernel se conecta a la consola del proceso
} pcb_t;

#define TAMANIO_PCB_SERIALIZADA 40 //sin contar la lista de instrucciones

// ACLARACION: todas las funciones devuelven 'true' en caso de exito. 'false' si hubo un error

bool sockets_enviar_pcb(int socket, pcb_t pcb, t_log* logger);
bool sockets_recibir_pcb(int socket, pcb_t *pcb_pointer, t_log* logger); //VER ACLARACION EN LA DEFINICION

#endif