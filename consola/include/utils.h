/*
 * utils.h
 *
 *  Created on: 14 abr. 2022
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

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


t_log* logger;
int kernel_socket;

void crear_logger();
t_config* cargar_config();
char* leer_valor_config(t_config*, char*);
int crear_conexion(char*, char*);
void finalizar_programa();

#endif /* UTILS_H_ */
