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
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"

#define MAX_STRING_SIZE 100
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

// Estructuras

typedef struct tabla_primer_nivel{
    int32_t cantidad_entradas; //de 1 a ENTRADAS_POR_TABLA (es el length de lista de entradas)
    t_list *lista_de_entradas; //es una lista de punteros a tabla_segundo_nivel
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
}
entrada_segundo_nivel;

// Variables Globales
t_config* config;
t_log* logger;
int kernel_socket;
char PUERTO_ESCUCHA[MAX_STRING_SIZE];
int32_t TAM_MEMORIA;
int32_t TAM_PAGINA;
int32_t ENTRADAS_POR_TABLA;
int32_t RETARDO_MEMORIA;
char ALGORITMO_REEMPLAZO[MAX_STRING_SIZE];
int32_t MARCOS_POR_PROCESO;
int32_t RETARDO_SWAP;
char PATH_SWAP[MAX_STRING_SIZE];

void* espacio_usuario;
t_list* lista_tabla_primer_nivel;

// Funciones
t_log* crear_logger(void);
void cargar_config();
void leer_config_string(t_config* , char* , char* );
void leer_config_int(t_config* , char* , uint32_t* );
//void finalizar_programa(int, t_log*);
void salir_error(t_log*, int*);

#endif
