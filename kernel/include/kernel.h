#include <stdio.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"


// *MACROS*

#define CONSOLA_BACKLOG 5
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

// *VARIABLES GLOBALES*

t_log * logger;
t_dictionary * pid_to_socket;
t_queue* instrucciones_buffer;

// *FUNCIONES*

void crear_logger(void);
void * escuchar_nuevas_consolas(void *);
void generar_pcb(void);
void finalizar_conexion_consola(uint32_t);
void inicializar_estructuras(void);
