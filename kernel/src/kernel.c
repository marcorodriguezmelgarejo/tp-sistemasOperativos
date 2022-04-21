#include "kernel.h"

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/kernel.log","Consola",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");

	return logger;
}

void escuchar_nuevas_consolas(t_log* logger){

    char buf[100];

    int sockfd = 0;

    int new_fd = 0;

    char instruccion_o_tamanio[12];

    unsigned long tamanio_proceso = 0;

    int enviar = OK_MESSAGE;

    sockets_abrir_servidor("8000", CONSOLA_BACKLOG, &sockfd, logger);

    while(1){
        sockets_esperar_cliente(sockfd, &new_fd, logger);

        do{
            sockets_recibir_string(new_fd, instruccion_o_tamanio, logger);
            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(new_fd, buf, logger);
                log_debug(logger, "Instruccion recibida: %s", buf);
            }
            else{
                sockets_recibir_dato(new_fd, &tamanio_proceso, sizeof(unsigned long), logger);
                log_debug(logger, "Tamanio de proceso recibido: %u", tamanio_proceso);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);
    }
}

int main()
{
    //crear diccionario que mapee pid de un pcb con el sock_fd de la consola que creó el proceso
    //crear lista de pcb
    //crear cola de new

    //inicializar un pcb por cada consola nueva y guardar instrucciones y tamanio
    //guardar el pcb en la cola de new

    //crear funcion que finalice la conexion con una consola pasandole el pid como parametro

    //crear funcion para salir del kernel cerrando los sockets, loggers, haciendo frees y todo eso

    t_log * logger = crear_logger();

    switch(fork()){

        case -1:
        log_error(logger, "Anduvo mal el fork(). Finalizando...");
        exit(ERROR_STATUS);

        case 0: // proceso hijo
        escuchar_nuevas_consolas(logger);

        default: // proceso padre
            while(1){continue;}
    }

    log_destroy(logger);

    return 0;
}
