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
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>


t_log* logger;

void crear_logger();
t_config* cargar_config();
char* leer_valor_config(t_config*, char*);
void finalizar_ejecucion(t_log*, t_config*);

#endif /* UTILS_H_ */
