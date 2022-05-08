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
#include <sys/time.h>

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

int consolas_socket;
int dispatch_socket;
int interrupt_socket;

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

int32_t contador_pid; //Es para determinar el pid del proximo proceso nuevo
pcb_t* todos_pcb; //Aca se guardan todos los pcb que esten presentes usando malloc
int32_t todos_pcb_length; //cuantos pcb se estan guardando en todos_pcb

int32_t grado_multiprogramacion_actual;

//En estas listas y colas se guardan direcciones de pcb en todos_pcb, no el pcb en si
t_queue* cola_new;
t_list* lista_ready;
t_list* lista_bloqueado;
t_list* lista_bloqueado_sus;
t_queue* cola_ready_sus;
pcb_t* en_ejecucion;

// *FUNCIONES*

void manejar_sigint(int);
void crear_logger(void);
void cargar_config(void);
void conectar_puerto_dispatch(void);
void conectar_puerto_interrupt(void);
void leer_str_config(t_config*, char*, char*, t_log*);
void leer_int_config(t_config*, char* value, int32_t*, t_log*);
void leer_float_config(t_config*, char*, float*, t_log*);
void * gestionar_nuevas_consolas(void *);
void finalizar_conexion_consola(int32_t);
void inicializar_estructuras(void);
void agregar_instruccion_a_lista(char **, char*);
void generar_pcb(char *, int32_t, int);
void actualizar_program_counter(pcb_t);
void actualizar_timestamp(pcb_t*);
uint64_t get_tiempo_transcurrido(uint64_t);
pcb_t* alocar_memoria_todos_pcb(void);
pcb_t* obtener_pcb_pointer(pcb_t);
pcb_t* obtener_pcb_pointer_desde_pid(int32_t);
void *gestionar_dispatch(void *);
void gestionar_proceso_a_io(void);
void gestionar_interrupcion_kernel(void);
bool es_algoritmo_srt(void);
void modificar_estimacion(pcb_t *);
void planificador_corto_plazo_ready(void);
pcb_t * seleccionar_proceso_menor_estimacion(void);
void planificador_corto_plazo_ejec(void);
void planificador_largo_plazo_ready(void);
void planificador_largo_plazo_exit(void);
void inicializar_estructuras_memoria(void);
void liberar_estructuras_memoria(void);
void liberar_memoria(void);

#endif