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

#include "../../shared/include/sockets.h"

/*
 *      Error codes:
 *      1 -> No se pudo crear el log
 *      2 -> No se pudo leer la config
 *      3 -> No se encontro el valor de IP_KERNEL
 *      4 -> No se encontro el valor de PUERTO_KERNEL
 *      5 -> Error al conectar con kernel
*/

#define MAX_STRING_SIZE 100

t_log* crear_logger();

void cargar_config(char *, char *, t_log*);

void leer_valor_config(t_config*, char*, char*, t_log*);

FILE* abrir_archivo_instrucciones(char *, t_log*);

void enviar_instrucciones(FILE*, int, t_log*);

void enviar_tamanio(unsigned long, int, t_log*);

void esperar_finalizacion(int, t_log*);

void finalizar_programa(int, t_log*);

void salir_error(int, t_log*, int*);

#endif /* FUNCIONES_H_ */
