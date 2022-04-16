/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "consola.h"

int main(int argc, char* argv[]) {

	char *ip, *port;
	//Etapa de inicializacion. Arrancamos log, config, pseudocode y conectamos con Kernel.
	crear_logger();
	t_config* config = cargar_config();

	if(argc != 3){
		log_error(logger, "No se ha ingresado la cantidad de parametros correcta. Terminando ejecucion.");
		exit(5);
	}

	ip = leer_valor_config(config, "IP_KERNEL");
	port = leer_valor_config(config, "PUERTO_KERNEL");
	config_destroy(config); //A partir de aca no usamos mas el archivo de config, entonces lo cerramos.

	log_info(logger, "Se han ingresado 2 parametros.");
	log_info(logger, "Filepath [Parametro 1]");
	log_info(logger, argv[1]);
	log_info(logger, "Tamanio memoria [Parametro 2]");
	log_info(logger, argv[2]);

	FILE* file_instrucciones = fopen(argv[1],"r");
	char* memory_size = argv[2];
	if(file_instrucciones == NULL){
		log_error(logger, "Error al abrir el archivo de instrucciones. Terminando ejecucion.");
		exit(6);
	} else {
		log_debug(logger, "Se ha abierto el archivo de instrucciones con exito.");
	}

	//TODO: check
	kernel_socket = crear_conexion(ip,port);

	// TODO
	//iniciar_servidor() ?
	// Hay que abrir una instancia de servidor tambien que escuche a mensajes que mande el modulo kernel.
	// Cuando mandamos un mensaje, queremos una especie de handshake de vuelta para confirmar que llego
	// el mensaje y que no hubo errores del otro lado.
	// Hay que hacerlo para cada linea que mandamos para no mandar instrucciones de mas mientras que esta
	// roto el modulo del otro lado.

	// TODO
	// enviar_tamanio(socket, argv[2]);
	read_and_send_to_kernel(file_instrucciones, kernel_socket);

	log_debug(logger, "Se ha terminado de ejecutar el programa. Finalizando.");
	//finalizar_programa();

	return EXIT_SUCCESS;
}

void read_and_send_to_kernel(FILE* file, int socket){
	char* line = string_new();
	size_t size = 0;
	int i = 1;
	while(getline(&line, &size, file) != -1){
		if(line[strlen(line)-1] == '\n'){
			line[strlen(line)-1] = '\0';
		}
		char* msg = string_new();
		char* nro = string_new();
		sprintf(nro,"%d",i);
		string_append(&msg,"Instruccion numero ");
		string_append(&msg,nro);
		log_info(logger,msg);
		log_info(logger,line);
		i++;
		//TODO
		// enviar_instruccion(socket, line);
		// int response_code = wait_for_response()
		// if(response_code != 1) exit(7); -> significaria error

	}
}
