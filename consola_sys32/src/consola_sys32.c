/*
 ============================================================================
 Name        : consola_sys32.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "consola_sys32.h"

int main(int argc, char* argv[]) {

	char *ip, *port;
	//Etapa de inicializacion. Arrancamos log, config, pseudocode y conectamos con Kernel.
	crear_logger();
	t_config* config = cargar_config();

	if(argc != 3){
		log_error(logger, "No se hay ingresado la cantidad de parametros correcta. Terminando ejecucion.");
	}
	log_info(logger, "Se han ingresado 2 parametros.");
	log_info(logger, "Filepath [Parametro 1]");
	log_info(logger, argv[1]);
	log_info(logger, "Tamanio memoria [Parametro 2]");
	log_info(logger, argv[2]);

	ip = leer_valor_config(config, "IP_KERNEL");
	port = leer_valor_config(config, "PUERTO_KERNEL");
	config_destroy(config);


	log_destroy(logger);
	return EXIT_SUCCESS;
}
