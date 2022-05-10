#include "funciones.h"

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/consola.log","consola_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		salir_error(NULL, NULL);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");

	return logger;
}

void cargar_config(char *ip, char* port, t_log* logger){

	t_config* config;

	if((config = config_create("./cfg/consola.config")) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		salir_error(logger, NULL);
	}

	leer_valor_config(config, "IP_KERNEL", ip, logger);
	leer_valor_config(config, "PUERTO_KERNEL", port, logger);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
}

void leer_valor_config(t_config* config, char* value, char* return_value, t_log* logger){
	
	if (config_has_property(config, value)){
		strcpy(return_value, config_get_string_value(config, value));
		log_info(logger, "%s: %s", value, return_value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		salir_error(logger, NULL);
	}
}

FILE* abrir_archivo_instrucciones(char * path, t_log* logger){

	FILE *file_instrucciones = fopen(path,"r");

	if(file_instrucciones == NULL){
		log_error(logger, "Error al abrir el archivo de instrucciones. Terminando ejecucion.");
		salir_error(logger, NULL);
	} else {
		log_debug(logger, "Se ha abierto el archivo de instrucciones con exito.");
	}

	return file_instrucciones;
}

//testeado (para instrucciones con un parametro)
int primer_parametro(char* line){
	int i = 0;
	int j = 0;
	char primer_parametro[MAX_STRING_SIZE];
	while(line[i] != ' '){
		i++;
	}
	i++;
	while(isdigit(line[i])){
		primer_parametro[j] = line[i];
		i++;
		j++;
	}
	primer_parametro[j]=0;
	return atoi(primer_parametro);
}

void enviar_instruccion(char* instruccion, int socket, t_log* logger){
	if (sockets_enviar_string(socket, "INSTRUCCION", logger) == false){
		log_error(logger, "Error al comunicarse con kernel. Finalizando...");
		salir_error(logger, &socket);
	}

	if (sockets_enviar_string(socket, instruccion, logger) == false){
		log_error(logger, "Error al comunicarse con kernel. Finalizando...");
		salir_error(logger, &socket);
	}
}

void enviar_linea(char* line, int socket, t_log* logger){
	// obtiene la operacion
	char nombre_operacion[MAX_STRING_SIZE];
	int largo_nombre_operacion;
	largo_nombre_operacion = strcspn(line, " ");
	strncpy(nombre_operacion, line, largo_nombre_operacion);
	nombre_operacion[largo_nombre_operacion] = 0;
	
	// envia n operaciones no_op si la operacion es no_op, de lo contrario, envia la operacion con los parametros
	if(strcmp(nombre_operacion, "NO_OP") == 0){
		int cantidad_noop = primer_parametro(line);
		for(int i = 0; i < cantidad_noop; i++){
			enviar_instruccion("NO_OP", socket, logger);
			log_debug(logger, "Enviando instruccion NO_OP numero %d", i);
		}
	}
	else{
		enviar_instruccion(line, socket, logger);
	}
}

void enviar_instrucciones(FILE* file, int socket, t_log* logger){

	char line[MAX_STRING_SIZE];
	int i = 1;

	while(fgets(line, MAX_STRING_SIZE, file) != NULL){
		
		line[strcspn(line, "\n")] = '\0';

		log_debug(logger,"Instruccion numero %d: %s", i, line);

		enviar_linea(line, socket, logger);

		i++;

	}

	log_info(logger, "Se han enviado con exito las instrucciones al kernel");
}

void enviar_tamanio(int32_t tamanio, int socket, t_log* logger){
	
	if (sockets_enviar_string(socket, "TAMANIO", logger) == false){
		log_error(logger, "Error al comunicarse con kernel. Finalizando...");
		salir_error(logger, &socket);
	}

	if (sockets_enviar_dato(socket, &tamanio, sizeof(int32_t), logger) == false){
		log_error(logger, "Error al comunicarse con kernel. Finalizando...");
		salir_error(logger, &socket);
	}
}

void esperar_finalizacion(int socket, t_log* logger){

	int32_t msg = 0;

	log_info(logger, "Esperando mensaje de finalizacion por parte del kernel...");

	if (sockets_recibir_dato(socket, &msg, sizeof(int32_t), logger) == false){
		log_error(logger, "Error al comunicarse con kernel. Finalizando...");
		salir_error(logger, &socket);
	}

}

void finalizar_programa(int kernel_socket, t_log* logger){
	sockets_cerrar(kernel_socket);
	log_destroy(logger);
}

void salir_error(t_log* logger, int* kernel_socket_pointer){
	if (kernel_socket_pointer != NULL) sockets_cerrar(*kernel_socket_pointer);
	if (logger != NULL) log_destroy(logger);
	exit(ERROR_STATUS);
}