#include "kernel.h"

void crear_logger(void){

	if((logger = log_create(LOG_FILENAME,"kernel_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");
}

void * escuchar_nuevas_consolas(void * arg){

    char buffer[100];
    int socket_server = 0;
    int socket_consola = 0;
    char instruccion_o_tamanio[12];
    int32_t tamanio_proceso = 0;
    t_queue* instrucciones_buffer = queue_create();

    sockets_abrir_servidor("8000", CONSOLA_BACKLOG, &socket_server, logger);

    while(1){
        sockets_esperar_cliente(socket_server, &socket_consola, logger);
        queue_clean(instrucciones_buffer);
        do{
            sockets_recibir_string(socket_consola, instruccion_o_tamanio, logger);
            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(socket_consola, buffer, logger);
                log_debug(logger, "Instruccion recibida: %s", buffer);
                queue_push(instrucciones_buffer, buffer);
            }
            else{
                sockets_recibir_dato(socket_consola, &tamanio_proceso, sizeof (int32_t), logger);
                log_debug(logger, "Tamanio de proceso recibido: %u", tamanio_proceso);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

        generar_pcb(instrucciones_buffer, tamanio_proceso, socket_consola);
    }

    return NULL;
}

void generar_pcb(t_queue* instrucciones_buffer, int32_t tamanio_proceso, int socket){

    char pid_string[12];

    pcb_t pcb_nuevo = {
    contador_pid,
    tamanio_proceso,
    0,
    list_create(),
    -1,
    ESTIMACION_INICIAL,
    };

    //TODO:
    //guardar instrucciones del instrucciones_buffer a la lista de instrucciones del pcb

    queue_push(cola_new, &pcb_nuevo);

    sprintf(pid_string, "%d", contador_pid); //para pasar de int a string (para usarse como key del diccionario 'pid_to_socket')
    dictionary_put(pid_to_socket, pid_string, &socket);

    contador_pid++;
}

void finalizar_conexion_consola(int32_t pid){

    int * socket_pointer = NULL;

    char pid_string[12];

    int32_t dummy = DUMMY_VALUE;

    sprintf(pid_string, "%d", pid);

    if ((socket_pointer = dictionary_get(pid_to_socket, pid_string)) == NULL){
        log_error(logger, "No se encontró el socket correspondiente al pid %u", pid);
        return;
    }

    if (sockets_enviar_dato(*socket_pointer, &dummy, sizeof(int32_t), logger) == false){
	    log_error(logger, "Error al comunicarse con consola pid %u", pid);
    }

    sockets_cerrar(*socket_pointer);

    log_info(logger, "Se cerró la conexion con consola pid %u", pid);
}

void inicializar_estructuras(void){

    pid_to_socket = dictionary_create();
    
    cola_new = queue_create();
    cola_ready = queue_create();
    lista_bloqueado = list_create();
    lista_bloqueado_sus = list_create();
    cola_ready_sus = queue_create();
    cola_exit = queue_create();

    crear_logger();
}

void cargar_config(void){
    //TODO:
    //implementar
    return;
}

int main(void)
{
    //TODO:
    //crear funcion para salir del kernel cerrando los sockets, loggers, haciendo frees y todo eso

    pthread_t h1;

    inicializar_estructuras();

    cargar_config();

    pthread_create(&h1, NULL, escuchar_nuevas_consolas, NULL);

    sleep(8);

    finalizar_conexion_consola(1);

    sleep(60);

    log_destroy(logger);

    return 0;
}
