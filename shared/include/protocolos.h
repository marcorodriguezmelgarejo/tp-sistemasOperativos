#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

#define OK_MESSAGE 0
#define ERROR_MESSAGE 1
#define DUMMY_VALUE 0 

typedef struct pcb_t{
    uint32_t pid;
    uint32_t tamanio;
    uint32_t program_counter;
    //FALTA LISTA DE INSTRUCCIONES. HAY QUE VER EL TIPO
    uint32_t tabla_paginas;
    float estimacion_rafaga;
} pcb_t;

#endif