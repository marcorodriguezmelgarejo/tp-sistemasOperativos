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
#include <semaphore.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"


// *MACROS*

#define CONSOLA_BACKLOG 5
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1
#define CONFIG_FILENAME "./cfg/kernel.config"
#define LOG_FILENAME "./cfg/kernel.log"
#define MAX_BUFFER_SIZE 100
#define MOTIVO_LENGTH 10 //para gestionar_dispatch()

// *VARIABLES GLOBALES*

pthread_t h1, h2;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_bloqueado;
pthread_mutex_t mutex_lista_bloqueado_suspendido;
pthread_mutex_t mutex_cola_ready_suspendido;
pthread_mutex_t mutex_en_ejecucion;
pthread_mutex_t mutex_grado_multiprogramacion_actual;
pthread_mutex_t mutex_interrupcion_cpu;
pthread_mutex_t mutex_cola_IO;
pthread_mutex_t mutex_en_IO;
pthread_mutex_t mutex_cola_instrucciones_memoria;

t_log * logger;

int consolas_socket;
int dispatch_socket;
int interrupt_socket;
int memoria_socket;

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

int32_t grado_multiprogramacion_actual;

//En estas listas y colas se guardan punteros a pcb, no el pcb en si
t_queue* cola_new;
t_list* lista_ready;
t_list* lista_bloqueado;
t_list* lista_bloqueado_suspendido;
t_queue* cola_ready_suspendido;
pcb_t* en_ejecucion;

typedef struct datos_tiempo_bloqueo{
    int32_t tiempo_bloqueo;
    pcb_t *pcb_pointer;
} datos_tiempo_bloqueo;

t_queue* cola_IO; // cola de 'datos_tiempo_bloqueo'. Son procesos que quieren entrar a IO
datos_tiempo_bloqueo en_IO; //se guarda los datos del proceso actualmente en IO

bool ya_se_envio_interrupcion_cpu;

typedef struct instruccion_memoria{
    pcb_t *pcb_pointer;
    int32_t codigo_operacion;
    sem_t * semaforo_pointer;
} instruccion_memoria;

// *FUNCIONES*
bool recibir_respuesta_memoria(void);
void solicitar_operacion_a_memoria(int32_t pid, int32_t motivo);
void enviar_fin_memoria(void);
void inicializar_variables_globales(void);
void manejar_sigint(int);
void crear_logger(void);
void cargar_config(void);
void conectar_puerto_dispatch(void);
void conectar_puerto_interrupt(void);
void conectar_puerto_memoria(void);
void enviar_pcb_cpu(pcb_t*);
void enviar_interrupcion_cpu(void);
void leer_str_config(t_config*, char*, char*, t_log*);
void leer_int_config(t_config*, char* value, int32_t*, t_log*);
void leer_float_config(t_config*, char*, float*, t_log*);
void * gestionar_nuevas_consolas(void *);
void finalizar_conexion_consola(pcb_t *);
void inicializar_estructuras(void);
void agregar_instruccion_a_lista(char **, char*);
pcb_t *generar_pcb(char *, int32_t, int);
void actualizar_program_counter_en_ejecucion(int32_t);
pcb_t* alocar_memoria_pcb(void);
void *gestionar_dispatch(void *);
bool es_algoritmo_srt(void);
void sumar_duracion_rafaga(pcb_t *, int32_t);
void transicion_consola_new(char *, int32_t, int);
void transicion_ejec_ready(int32_t);
int seleccionar_proceso_menor_estimacion(void);
void transicion_ready_ejec(void);
bool transicion_new_ready(void);
void transicion_ejec_exit(void);
void transicion_ejec_bloqueado(int32_t, int32_t);
void transicion_bloqueado_ready(pcb_t *);
void transicion_bloqueado_bloqueado_suspendido(pcb_t *);
void transicion_bloqueado_suspendido_ready_suspendido(pcb_t *);
bool transicion_ready_suspendido_ready();
int get_indice_pcb_pointer(t_list*, pcb_t*);
void inicializar_estructuras_memoria(pcb_t *);
void liberar_estructuras_memoria(pcb_t *);
void liberar_memoria(void);
void liberar_memoria_lista_pcb(t_list*);
void liberar_memoria_cola_pcb(t_queue*);
void liberar_memoria_pcb(pcb_t*);
void liberar_threads_cola(t_queue*);
void memoria_suspender_proceso(pcb_t*);
void memoria_finalizar_proceso(pcb_t*);
void ingresar_proceso_a_ready(void);
int32_t milisegundos_a_microsegundos (int32_t);
void si_es_necesario_enviar_interrupcion_o_ready_ejec(void);
void si_es_necesario_ready_ejec(void);
int32_t calcular_estimacion_rafaga(pcb_t *);
void enviar_fin_cpu(void);
void* hilo_timer(void*);
void si_se_puede_entrar_IO(void);
void entrar_IO(pcb_t *, int32_t);
void* hilo_timer_IO(void*);
void terminar_IO(void);

#endif