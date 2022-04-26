#include "funciones.h"

int main(int argc, char* argv[]) {

	char ip[MAX_STRING_SIZE], port[MAX_STRING_SIZE];
	t_log* logger = NULL;
	FILE* file_instrucciones = NULL;
	int kernel_socket = 0;

	logger = crear_logger();

	//chequear arg de entrada
	if(argc != 3){
		log_error(logger, "Uso: consola path tamanio");
		salir_error(logger, NULL);
	}

	cargar_config(ip, port, logger);

	log_info(logger, "Filepath = \"%s\"", argv[1]);
	log_info(logger, "Tamanio proceso = %s", argv[2]);

	file_instrucciones = abrir_archivo_instrucciones(argv[1], logger);

	if (sockets_conectar_como_cliente(port, ip, &kernel_socket, logger) == false){
		log_error(logger, "Error al intentar conectar con kernel. Finalizando...");
		salir_error(logger, NULL);
	}

	enviar_instrucciones(file_instrucciones, kernel_socket, logger);
	fclose(file_instrucciones);

	enviar_tamanio((int32_t) atoi(argv[2]), kernel_socket, logger);

	esperar_finalizacion(kernel_socket, logger);

	log_info(logger, "Se ha terminado de ejecutar el programa. Finalizando.");
	finalizar_programa(kernel_socket, logger);

	return SUCCESS_STATUS;
}
