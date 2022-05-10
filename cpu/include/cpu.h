#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"

// MACROS
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

// TIPOS    
typedef enum alg_reemplazo_tlb_t{
    FIFO, 
    LRU
} alg_reemplazo_tlb_t;

typedef enum operacion_t{
    NO_OP,
    I_O,
    READ,
    WRITE,
    COPY,
    EXIT,
} operacion_t;

typedef struct intruccion_t{
    operacion_t operacion;
    //parametros (no siempre se usan todos, cuales se usan depende de la op)
    int32_t tiempo_bloqueo, valor, dir_destino, dir_origen;
} instruccion_t;

// VARIABLES GLOBALES
pcb_t pcb;
t_log* logger;
// parametros del config
char IP_MEMORIA[16];
char PUERTO_MEMORIA[6];
char PUERTO_ESCUCHA_DISPATCH[6];
char PUERTO_ESCUCHA_INTERRUPT[6];
int32_t ENTRADAS_TLB;
alg_reemplazo_tlb_t REEEMPLAZO_TLB;
int32_t RETARDO_NOOP; //en ms

// FUNCIONES 
void cargar_config(t_log* logger);





#endif