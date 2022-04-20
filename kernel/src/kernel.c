#include "kernel.h"

//IMPORTANTE:
//ESTE ARCHIVO ES (POR AHORA) SOLO PARA TESTEAR LA CONSOLA

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/kernel.log","Consola",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.\n");

	return logger;
}


int main()
{
    int sockfd = 0;

    int new_fd = 0;

    char buf[100];

    char instruccion_o_tamanio[20];

    unsigned int tamanio_proceso = 0;

    int enviar = OK_MESSAGE;

    t_log * logger = crear_logger();

    sockets_abrir_servidor("8000", 5, &sockfd, logger);

    sockets_esperar_cliente(sockfd, &new_fd, logger);

    do{
        sockets_recibir_string(new_fd, instruccion_o_tamanio, logger);
        if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
            sockets_recibir_string(new_fd, buf, logger);
            printf("Instruccion recibida: %s\n", buf);
        }
        else{
            sockets_recibir_dato(new_fd, &tamanio_proceso, sizeof(unsigned long), logger);
            printf("Tamanio de proceso recibido: %u\n", tamanio_proceso);
        }
    }
    while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

    sockets_enviar_dato(new_fd, &enviar, sizeof(int), logger);

    sockets_cerrar(new_fd);
    sockets_cerrar(sockfd);

    log_destroy(logger);

    return 0;
}