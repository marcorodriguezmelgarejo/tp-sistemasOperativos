/*
 * utils.c
 *
 *  Created on: 14 abr. 2022
 *      Author: utnso
 *
 *      Error codes:
 *      1 -> No se pudo crear el log
 *      2 -> No se pudo leer la config
 *      3 -> No se encontro el valor de IP_KERNEL
 *      4 -> No se encontro el valor de PUERTO_KERNEL
 */

#include "utils.h"

void crear_logger(){

	if((logger = log_create("./consola.log","Consola",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.\n");
}

t_config* cargar_config(){

	t_config* config;

	if((config = config_create("./consola.config")) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		exit(2);
	}

	log_debug(logger, "Se ha leido el archivo de config con exito.");
	return config;
}

char* leer_valor_config(t_config* config, char* value){

	char* reading = string_new();
	string_append(&reading, "Leyendo valor de ");
	string_append(&reading, value);
	log_info(logger, reading);

	char* return_value = string_new();
	if (config_has_property(config, value)){
		return_value = config_get_string_value(config, value);
		char* string = string_new();
		string_append(&string, value);
		string_append(&string, ": ");
		string_append(&string ,return_value);
		log_info(logger, string);
	} else {
		log_error(logger, "No se encontro valor de IP_KERNEL. Finalizando ejecucion.");
		char* string = string_new();
		string_append(&string, "No se encontro valor de ");
		string_append(&string, value);
		string_append(&string, ". Finalizando ejecucion.");
		log_error(logger, string);
		exit(3);
	}

	return return_value;
}

