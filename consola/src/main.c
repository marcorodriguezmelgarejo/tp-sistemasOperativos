#include "funciones.h"

int main(int argc, char* argv[]) {

	char *ip, *port;
	t_log* logger = NULL;
	FILE* file_instrucciones = NULL;
	char * memory_size = NULL;
	int kernel_socket;

	logger = crear_logger();

	//chequear arg de entrada
	if(argc != 3){
		log_error(logger, "Uso: consola path tamanio");
		exit(5);
	}

	cargar_config(&ip, &port, logger);

	log_info(logger, "Se han ingresado 2 parametros:");
	log_info(logger, "Filepath = \"%s\"", argv[1]);
	log_info(logger, "Tamanio memoria = %s", argv[2]);

	file_instrucciones = abrir_archivo_instrucciones(argv[1], logger);

	memory_size = argv[2];

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
	read_and_send_to_kernel(file_instrucciones, kernel_socket, logger);

	log_debug(logger, "Se ha terminado de ejecutar el programa. Finalizando.");
	finalizar_programa(kernel_socket, logger);

	return EXIT_SUCCESS;
}
