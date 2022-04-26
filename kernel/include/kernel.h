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
#define CONFIG_FILENAME "./cfg/kernel.config"
#define LOG_FILENAME "./cfg/kernel.log"

// *VARIABLES GLOBALES*

t_log * logger;
t_dictionary * pid_to_socket; //mapea pid (tiene que ser un string) de un proceso al socket de la consola correspondiente

char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *IP_CPU;
char *PUERTO_CPU_DISPATCH;
char *PUERTO_CPU_INTERRUPT;
char *PUERTO_ESCUCHA;
char *ALGORITMO_PLANIFICACION; //"FIFO" o "SRT"
int32_t ESTIMACION_INICIAL;
float ALFA;
int32_t GRADO_MULTIPROGRAMACION;
int32_t TIEMPO_MAXIMO_BLOQUEADO;

int32_t contador_pid; //Es para determinar el pid del proximo proceso nuevo

t_queue* cola_new;
t_queue* cola_ready;
t_list* lista_bloqueado;
t_list* lista_bloqueado_sus;
t_queue* cola_ready_sus;
t_queue* cola_exit;
pcb_t* proceso_ejec;

// *FUNCIONES*

void crear_logger(void);
void * escuchar_nuevas_consolas(void *);
void finalizar_conexion_consola(int32_t);
void inicializar_estructuras(void);
