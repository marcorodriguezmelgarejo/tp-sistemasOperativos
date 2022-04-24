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

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolos.h"

#define MAX_STRING_SIZE 100

#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

t_log* crear_logger(void);

void cargar_config(char *, char *, t_log*);

void leer_valor_config(t_config*, char*, char*, t_log*);

FILE* abrir_archivo_instrucciones(char *, t_log*);

void enviar_instrucciones(FILE*, int, t_log*);

void enviar_tamanio(uint32_t, int, t_log*);

void esperar_finalizacion(int, t_log*);

void finalizar_programa(int, t_log*);

void salir_error(t_log*, int*);

#endif /* FUNCIONES_H_ */
