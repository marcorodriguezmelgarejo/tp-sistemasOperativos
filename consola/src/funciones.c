#include "funciones.h"

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/consola.log","Consola",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.\n");

	return logger;
}

void cargar_config(char ** ip, char** port, t_log* logger){

	t_config* config;

	if((config = config_create("./cfg/consola.config")) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		exit(2);
	}

	*ip = leer_valor_config(config, "IP_KERNEL", logger);
	*port = leer_valor_config(config, "PUERTO_KERNEL", logger);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
}

char* leer_valor_config(t_config* config, char* value, t_log* logger){

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

FILE* abrir_archivo_instrucciones(char * path, t_log* logger){

	FILE *file_instrucciones = fopen(path,"r");

	if(file_instrucciones == NULL){
		log_error(logger, "Error al abrir el archivo de instrucciones. Terminando ejecucion.");
		exit(6);
	} else {
		log_debug(logger, "Se ha abierto el archivo de instrucciones con exito.");
	}

	return file_instrucciones;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void read_and_send_to_kernel(FILE* file, int socket, t_log* logger){

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

void finalizar_programa(int kernel_socket, t_log* logger){
	close(kernel_socket);
	log_destroy(logger);
}
