#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"


// *MACROS*

#define CONSOLA_BACKLOG 5
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1
#define CONFIG_FILENAME "./cfg/kernel.config"
#define LOG_FILENAME "./cfg/kernel.log"
#define MAX_BUFFER_SIZE 100

// *VARIABLES GLOBALES*

pthread_t h1, h2;
t_log * logger;
t_dictionary * pid_to_socket; //mapea pid (tiene que ser un string) de un proceso al socket de la consola correspondiente

int consolas_socket = 0;
int dispatch_socket = 0;

char IP_MEMORIA[16];
char PUERTO_MEMORIA[6];
char IP_CPU[16];
char PUERTO_CPU_DISPATCH[6];
char PUERTO_CPU_INTERRUPT[6];
char PUERTO_ESCUCHA[6];
char ALGORITMO_PLANIFICACION[5]; //"FIFO" o "SRT"
int32_t ESTIMACION_INICIAL;
float ALFA;
int32_t GRADO_MULTIPROGRAMACION;
int32_t TIEMPO_MAXIMO_BLOQUEADO;

int32_t contador_pid = 0; //Es para determinar el pid del proximo proceso nuevo
pcb_t* todos_pcb = NULL; //Aca se guardan todos los pcb que esten presentes usando malloc
int32_t todos_pcb_length = 0; //cuantos pcb se estan guardando en todos_pcb

//En estas listas y colas se guardan direcciones de pcb en array_pcb, no el pcb en si
t_queue* cola_new;
t_queue* cola_ready;
t_list* lista_bloqueado;
t_list* lista_bloqueado_sus;
t_queue* cola_ready_sus;
t_queue* cola_exit;
pcb_t* proceso_ejec;

// *FUNCIONES*

void manejar_sigint(int);
void crear_logger(void);
void leer_str_config(t_config*, char*, char*, t_log*);
void leer_int_config(t_config*, char* value, int32_t*, t_log*);
void leer_float_config(t_config*, char*, float*, t_log*);
void * gestionar_nuevas_consolas(void *);
void finalizar_conexion_consola(int32_t);
void inicializar_estructuras(void);
void agregar_instruccion_a_lista(char **, char*);
void generar_pcb(char *, int32_t, int);
void actualizar_pcb(pcb_t);
pcb_t* alocar_memoria_todos_pcb(void);
void liberar_memoria(void);

#endif