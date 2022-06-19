#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"

// MACROS
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1
#define PID_NULO -1

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

typedef struct dir_logica_t{
int32_t entrada_tabla_1er_nivel;//esta en pcb
int32_t entrada_tabla_2do_nivel;
int32_t desplazamiento;
} dir_logica_t;

typedef struct tlb_entrada_t{
int32_t pagina;
int32_t marco;
} tlb_entrada_t;

typedef struct tlb_t{
t_list * entradas; 
} tlb_t;

// VARIABLES GLOBALES
pcb_t en_ejecucion;
t_log* logger;
uint64_t timestamp_comienzo_rafaga;
int32_t entradas_tabla_paginas;
int32_t tamanio_pagina;
bool interrupcion; // si llego una interrupcion desde el Kernel
bool finalizar; // si se debe finalizar la ejecucion al final del ciclo
// parametros del config
char IP_MEMORIA[16];
char PUERTO_MEMORIA[6];
char PUERTO_ESCUCHA_DISPATCH[6];
char PUERTO_ESCUCHA_INTERRUPT[6];
int32_t ENTRADAS_TLB;
alg_reemplazo_tlb_t REEEMPLAZO_TLB;
int32_t RETARDO_NOOP; //en ms
// conexiones
int dispatch_socket;
int interrupt_socket;
int memoria_socket;
// semaforos
sem_t PCB_en_CPU;
sem_t CPU_vacia;
pthread_mutex_t mutex_interrupcion;
pthread_mutex_t mutex_PCB;
// hilos
pthread_t hilo_dispatch;
pthread_t hilo_ciclo_instruccion;
pthread_t hilo_interrupcion;


// FUNCIONES 
void cargar_config(t_log* logger);
void crear_logger();
void loguear_PC();
instruccion_t decode(char* string_instruccion);
operacion_t decode_operacion(char* string_instruccion);
void chequear_interrupcion();
bool desalojar_y_devolver_pcb(char * motivo);
bool devolver_pcb(char* motivo, int32_t tiempo_rafaga);
void desalojar_pcb();
bool conectar_con_kernel();
bool conectar_dispatch();
bool conectar_interrupt();
bool execute(instruccion_t instruccion);
void instruccion_siguiente(char* retorno);
void fetch(char* string_instruccion);
int primer_parametro(char* line);
int segundo_parametro(char* line);
operacion_t decode_operacion(char* string_instruccion);
instruccion_t decode(char* string_instruccion);
bool inicializar_semaforos();
void esperar_pcb();
void esperar_interrupcion();
void ciclo_instruccion();
bool no_op();
bool salir();
bool i_o(int32_t tiempo_bloqueo);
uint64_t actualizar_timestamp();
int32_t get_tiempo_transcurrido(uint64_t timestamp_anterior);
void recibir_interrupcion_del_kernel();
void matar_kernel();
bool handshake_memoria();
void conectar_con_memoria();
bool leer(int32_t dir_origen);
bool escribir(int32_t dir_destino, int32_t valor);
bool leer_dir_logica(int32_t direccion_logica, int32_t *valor_leido);
bool escribir_dir_logica(int32_t direccion_logica, int32_t valor);
bool leer_dir_fisica(int32_t direccion_fisica, int32_t *puntero_valor_leido);
bool escribir_dir_fisica(int32_t direccion_fisica, int32_t *puntero_valor_leido);
tlb_entrada_t buscar_pagina(int32_t pagina);
int calcular_numero_pagina(int direccion_logica);
int32_t calcular_entrada_tabla_1er_nivel(int numero_pagina);
int32_t calcular_entrada_tabla_2do_nivel(int numero_pagina);
int calcular_desplazamiento(int direccion_logica, int numero_pagina);
bool fetch_operandos(instruccion_t *instruccion);

#endif