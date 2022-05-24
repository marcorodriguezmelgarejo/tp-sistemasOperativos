#ifndef FUNCIONES_H_
#define FUNCIONES_H_

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

typedef struct {
    uint32_t id; // Identificador
    void* start; // Primera locacion de memoria 
    void* end; // Ultima locacion de memoria 
    bool used; //Si esta en uso o no
} t_pagina;

// Variables Globales
t_config* config;
t_log* logger;
int kernel_socket;
char PUERTO_ESCUCHA[MAX_STRING_SIZE];
uint32_t TAM_MEMORIA;
uint32_t TAM_PAGINA;
uint32_t ENTRADAS_POR_TABLA;
uint32_t RETARDO_MEMORIA;
char ALGORITMO_REEMPLAZO[MAX_STRING_SIZE];
uint32_t MARCOS_POR_PROCESO;
uint32_t RETARDO_SWAP;
char PATH_SWAP[MAX_STRING_SIZE];

// Funciones
t_log* crear_logger(void);
void cargar_config();
void leer_config_string(t_config* , char* , char* );
void leer_config_int(t_config* , char* , uint32_t* );
//void finalizar_programa(int, t_log*);
void salir_error(t_log*, int*);

#endif /* FUNCIONES_H_ */
