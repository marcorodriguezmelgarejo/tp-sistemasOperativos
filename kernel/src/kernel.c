#include "kernel.h"

void crear_logger(void){

	if((logger = log_create("./cfg/kernel.log","Consola",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");
}

void * escuchar_nuevas_consolas(void * arg){

    char buffer[100];

    int sockfd = 0;

    int new_fd = 0;

    char instruccion_o_tamanio[12];

    uint32_t tamanio_proceso = 0;

    queue_clean(instrucciones_buffer);

    sockets_abrir_servidor("8000", CONSOLA_BACKLOG, &sockfd, logger);

    while(1){
        sockets_esperar_cliente(sockfd, &new_fd, logger);

        do{
            sockets_recibir_string(new_fd, instruccion_o_tamanio, logger);
            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(new_fd, buffer, logger);
                log_debug(logger, "Instruccion recibida: %s", buffer);
                queue_push(instrucciones_buffer, buffer);
            }
            else{
                sockets_recibir_dato(new_fd, &tamanio_proceso, sizeof (uint32_t), logger);
                log_debug(logger, "Tamanio de proceso recibido: %u", tamanio_proceso);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

        dictionary_put(pid_to_socket, "1", &new_fd); //hacer esto en generar_pcb()

        generar_pcb();
    }

    return NULL;
}

void generar_pcb(void){
    //implementar
    return;
}

void finalizar_conexion_consola(uint32_t pid){

    int * socket_pointer = NULL;

    char pid_string[12];

    uint32_t dummy = DUMMY_VALUE;

    sprintf(pid_string, "%d", pid);

    if ((socket_pointer = dictionary_get(pid_to_socket, pid_string)) == NULL){
        log_error(logger, "No se encontró el socket correspondiente al pid %u", pid);
        return;
    }

    if (sockets_enviar_dato(*socket_pointer, &dummy, sizeof(uint32_t), logger) == false){
	    log_error(logger, "Error al comunicarse con consola pid %u", pid);
    }

    sockets_cerrar(*socket_pointer);

    log_info(logger, "Se cerró la conexion con consola pid %u", pid);
}

void inicializar_estructuras(void){

    pid_to_socket = dictionary_create();

    instrucciones_buffer = queue_create();

    crear_logger();
}

int main(void)
{
    //crear lista de pcb
    //crear cola de new

    //inicializar un pcb por cada consola nueva y guardar instrucciones y tamanio
    //guardar el pcb en la cola de new

    //crear funcion para salir del kernel cerrando los sockets, loggers, haciendo frees y todo eso

    pthread_t h1;

    inicializar_estructuras();

    pthread_create(&h1, NULL, escuchar_nuevas_consolas, NULL);

    sleep(8);

    finalizar_conexion_consola(1);

    sleep(60);

    log_destroy(logger);

    return 0;
}
