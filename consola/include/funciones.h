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

/*
 *      Error codes:
 *      1 -> No se pudo crear el log
 *      2 -> No se pudo leer la config
 *      3 -> No se encontro el valor de IP_KERNEL
 *      4 -> No se encontro el valor de PUERTO_KERNEL
*/

t_log* crear_logger();
void cargar_config(char **, char **, t_log*);
char* leer_valor_config(t_config*, char*, t_log*);
FILE* abrir_archivo_instrucciones(char *, t_log*);
int crear_conexion(char*, char*);
void finalizar_programa(int, t_log*);
void read_and_send_to_kernel(FILE*, int, t_log*);

#endif /* FUNCIONES_H_ */
