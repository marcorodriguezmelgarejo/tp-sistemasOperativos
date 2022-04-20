#include "funciones.h"

int main(int argc, char* argv[]) {

	char ip[MAX_STRING_SIZE], port[MAX_STRING_SIZE];
	t_log* logger = NULL;
	FILE* file_instrucciones = NULL;
	char * memory_size = NULL;
	int kernel_socket = 0;

	logger = crear_logger();

	//chequear arg de entrada
	if(argc != 3){
		log_error(logger, "Uso: consola path tamanio");
		salir_error(5, logger, NULL);
	}

	cargar_config(ip, port, logger);

	log_info(logger, "Se han ingresado 2 parametros:");
	log_info(logger, "Filepath = \"%s\"", argv[1]);
	log_info(logger, "Tamanio proceso = %s", argv[2]);

	file_instrucciones = abrir_archivo_instrucciones(argv[1], logger);

	if (sockets_conectar_como_cliente(port, ip, &kernel_socket, logger) == false){
		log_error(logger, "Error al intentar conectar con kernel. Finalizando...");
		salir_error(7, logger, NULL);
	}

	// TODO
	//iniciar_servidor() ?
	// Hay que abrir una instancia de servidor tambien que escuche a mensajes que mande el modulo kernel.
	// Cuando mandamos un mensaje, queremos una especie de handshake de vuelta para confirmar que llego
	// el mensaje y que no hubo errores del otro lado.
	// Hay que hacerlo para cada linea que mandamos para no mandar instrucciones de mas mientras que esta
	// roto el modulo del otro lado.

	enviar_instrucciones(file_instrucciones, kernel_socket, logger);
	enviar_tamanio(strtoul(argv[2], NULL, 10), kernel_socket, logger);

	esperar_finalizacion(kernel_socket, logger);

	log_debug(logger, "Se ha terminado de ejecutar el programa. Finalizando.");
	finalizar_programa(kernel_socket, logger);

	return EXIT_SUCCESS;
}
