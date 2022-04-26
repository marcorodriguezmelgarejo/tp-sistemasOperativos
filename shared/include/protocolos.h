#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

#include <stdint.h>
#include <commons/collections/queue.h> 
#include <commons/collections/list.h> 
#include <commons/collections/dictionary.h>

#define OK_MESSAGE 0
#define ERROR_MESSAGE 1
#define DUMMY_VALUE 0 

#define MAX_INSTRUCCION_SIZE 50

typedef struct pcb_t{
    int32_t pid;
    int32_t tamanio;
    int32_t program_counter;
    char ** lista_instrucciones;
    int32_t tabla_paginas;
    int32_t estimacion_rafaga;
} pcb_t;

#endif