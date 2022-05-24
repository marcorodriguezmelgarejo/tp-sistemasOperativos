#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include <ctype.h>

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
    INVALIDA=0,
    NO_OP=1,
    I_O=2,
    READ=3,
    WRITE=4,
    COPY=5,
    EXIT=6,
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
void crear_logger();
instruccion_t decode(char* string_instruccion);
operacion_t decode_operacion(char* string_instruccion);



#endif