#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"

// Macros

#define MAX_STRING_SIZE 100
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

#define CREAR_ARCHIVO_SWAP 0
#define TRASLADAR_PAGINA_A_DISCO 1
#define TRASLADAR_PAGINA_A_MEMORIA 2
#define TRASLADAR_PROCESO_A_DISCO 3
#define BORRAR_ARCHIVO_SWAP 4

// Estructuras

typedef struct tabla_primer_nivel{
    int32_t pid;
    int32_t cantidad_entradas; //de 1 a ENTRADAS_POR_TABLA (es el length de lista de entradas)
    int32_t tamanio_conjunto_residente; //de 0 a MARCOS_POR_PROCESO
    int32_t puntero_clock; //guarda un indice de la lista_paginas_cargadas
    t_list *lista_paginas_cargadas;//lista de paginas con presencia ordenadas por orden ascendente de carga(para algo de reemplazo)
    int32_t cantidad_paginas; //cantidad de paginas totales del proceso (es igual al tamanio del archivo swap / TAM_PAGINA)
    t_list *lista_de_tabla_segundo_nivel; //es una lista de punteros a tabla_segundo_nivel
} tabla_primer_nivel;

typedef struct tabla_segundo_nivel{
    int32_t cantidad_entradas; //de 1 a ENTRADAS_POR_TABLA (es el length de lista de entradas)
    t_list *lista_de_entradas; //es una lista de punteros a 'entrada_segundo_nivel'
} tabla_segundo_nivel;

typedef struct entrada_segundo_nivel{
    int32_t numero_marco;
    bool presencia;
    bool usado;
    bool modificado;
} entrada_segundo_nivel;

typedef struct instruccion_swap{
    int32_t numero_instruccion;
    int32_t pid;
    int32_t tamanio_proceso;
    int32_t numero_pagina;
    int32_t numero_marco;
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    sem_t * semaforo_pointer; //un semaforo para dar la señal de cuando se termina de ejecutar la instruccion swap
} instruccion_swap;

// Variables Globales

char PUERTO_ESCUCHA[MAX_STRING_SIZE];
int32_t TAM_MEMORIA;
int32_t TAM_PAGINA;
int32_t ENTRADAS_POR_TABLA;
int32_t RETARDO_MEMORIA;
char ALGORITMO_REEMPLAZO[MAX_STRING_SIZE];
int32_t MARCOS_POR_PROCESO;
int32_t RETARDO_SWAP;
char PATH_SWAP[MAX_STRING_SIZE];

pthread_t h1, h2, h3;
pthread_mutex_t mutex_cola_instrucciones_swap;
pthread_mutex_t mutex_conexiones;
sem_t contador_cola_instrucciones_swap;

t_config* config;
t_log* logger;

int kernel_socket;
int cpu_socket;

int32_t cantidad_total_marcos;

void* espacio_usuario;

void* bitarray_aux;
t_bitarray * marcos_libres;

t_queue* cola_instrucciones_swap;

t_dictionary * diccionario_tabla_pointers;

// Funciones
void mostrar_cantidad_marcos_libres(void);
void atender_fin_memoria(void);
void test_algoritmo_reemplazo(void);
void agregar_pagina(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, bool usado);
void cambiar_puntero_clock(tabla_primer_nivel* tabla_pointer, int32_t indice_quitado);
bool puntero_es_ultimo_elemento_o_excede_lista(tabla_primer_nivel *tabla_pointer);
int32_t get_pagina_apuntada_lista_paginas_cargadas(tabla_primer_nivel* tabla_pointer);
void escribir_ceros_archivo(FILE * archivo, int32_t cantidad_ceros);
void inicializar_valores_tabla_primer_nivel(tabla_primer_nivel* tabla_primer_nivel_pointer, int32_t pid, int32_t cantidad_paginas, int32_t cantidad_entradas_primer_nivel);
void entrada_segundo_nivel_setear_bits_al_traer_a_memoria(entrada_segundo_nivel* entrada_segundo_nivel_pointer, int32_t numero_marco);
void enviar_instruccion_swap_CREAR_ARCHIVO_SWAP(int32_t pid, int32_t tamanio_proceso);
void enviar_instruccion_swap_BORRAR_ARCHIVO_SWAP(int32_t pid);
void enviar_instruccion_swap_TRASLADAR_PAGINA_A_DISCO(int32_t pid, int32_t numero_pagina, int32_t numero_marco);
void enviar_instruccion_swap_TRASLADAR_PAGINA_A_MEMORIA(int32_t pid, int32_t numero_pagina, int32_t numero_marco);
void enviar_instruccion_swap_TRASLADAR_PROCESO_A_DISCO(tabla_primer_nivel * tabla_pointer);
void agregar_pagina_lista_paginas_cargadas(tabla_primer_nivel *tabla_pointer, int32_t numero_pagina);
void quitar_pagina_lista_paginas_cargadas(tabla_primer_nivel *tabla_pointer, int32_t numero_pagina);
void limpiar_lista_paginas_cargadas(tabla_primer_nivel* tabla_pointer);
void iniciar_test_acciones(void);
void iniciar_test_algoritmo_reemplazo(void);
void test_acceder_tabla_primer_nivel(tabla_primer_nivel* tabla_pointer);
void borrar_entrada_diccionario_tabla_pointers(int32_t pid);
void crear_entrada_diccionario_tabla_pointers(int32_t pid, tabla_primer_nivel* tabla_pointer);
int32_t milisegundos_a_microsegundos(int32_t milisegundos);
tabla_primer_nivel* test_inicializar_proceso(void);
t_log* crear_logger(void);
void cargar_config();
void leer_config_string(t_config* , char* , char* );
void leer_config_int(t_config* , char* , int32_t* );
void crear_hilos(void);
void conectar_cpu_y_kernel(void);
void* hilo_escuchar_cpu(void * arg);
void* hilo_escuchar_kernel(void * arg);
tabla_primer_nivel* inicializar_proceso(int32_t pid, int32_t tamanio_proceso);
tabla_primer_nivel* crear_tabla_paginas_proceso(int32_t pid, int32_t cantidad_paginas, int32_t cantidad_entradas_primer_nivel, int32_t cantidad_entradas_segundo_nivel_ultima_entrada);
void liberar_memoria_tabla_proceso(tabla_primer_nivel* tabla_pointer);
void suspender_proceso(tabla_primer_nivel*);
void finalizar_proceso(tabla_primer_nivel*);
int32_t acceder_tabla_primer_nivel(tabla_primer_nivel* tabla_pointer, int32_t indice);
entrada_segundo_nivel* get_entrada_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina);
int32_t elegir_pagina_para_reemplazar(tabla_primer_nivel* tabla_pointer);
int32_t elegir_marco_libre(tabla_primer_nivel* tabla_pointer);
void acciones_trasladar_pagina_a_memoria(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco);
int32_t acceder_tabla_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_tabla_segundo_nivel, int32_t numero_pagina_solicitada);
int32_t acceder_espacio_usuario_lectura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento);
bool acceder_espacio_usuario_escritura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento, int32_t valor);
void *hilo_swap(void *arg);
void crear_archivo_swap(int32_t pid, int32_t tamanio_proceso);
void trasladar_pagina_a_disco(int32_t, int32_t, int32_t);
void trasladar_pagina_a_memoria(int32_t, int32_t, int32_t);
void trasladar_proceso_a_disco(tabla_primer_nivel*);
void borrar_archivo_swap(int32_t pid);
void inicializar_variables_globales(void);
void enviar_instruccion_swap(instruccion_swap);
int get_indice_lista_int32(t_list* lista, int32_t elemento_buscado);
void esperar_conexion_cpu(int socket_escucha);
void esperar_conexion_kernel(int socket_escucha);
bool handshake_cpu();
bool excede_la_tabla(tabla_primer_nivel* tabla_pointer, int32_t indice);
void liberar_memoria(void);
void manejar_sigint(int);
bool es_algoritmo_reemplazo_clock(void);
int32_t algoritmo_reemplazo_clock(tabla_primer_nivel* tabla_pointer);
int32_t algoritmo_reemplazo_clock_mejorado(tabla_primer_nivel* tabla_pointer);
int32_t clock_mejorado_primer_paso(tabla_primer_nivel * tabla_pointer);
int32_t clock_mejorado_segundo_paso(tabla_primer_nivel * tabla_pointer);
void algoritmo_reemplazo_actualizar_puntero(tabla_primer_nivel* tabla_pointer);
tabla_primer_nivel* obtener_tabla_con_pid(int32_t pid);
void atender_finalizacion_proceso(int32_t pid);
void atender_suspension_proceso(int32_t pid);
void atender_inicializacion_proceso(int32_t pid);
bool atender_acceso_tabla_segundo_nivel(void);
bool atender_acceso_lectura_espacio_usuario(void);
bool atender_acceso_escritura_espacio_usuario(void);
bool atender_acceso_tabla_primer_nivel(void);
void marcar_marcos_como_libres_proceso(tabla_primer_nivel* tabla_pointer);
void marcar_marco_como_libre(int32_t numero_marco);
void marcar_marco_como_ocupado(int32_t numero_marco);
int32_t* get_puntero_a_entero_de_espacio_usuario(int32_t numero_marco, int32_t desplazamiento);
void poner_bit_usado_true(tabla_primer_nivel * tabla_pointer, int32_t numero_pagina);
void poner_bit_modificado_true(tabla_primer_nivel * tabla_pointer, int32_t numero_pagina);
#endif
