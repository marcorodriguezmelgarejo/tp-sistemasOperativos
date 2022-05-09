#include "kernel.h"

void finalizar_conexion_consola(int32_t pid){
    /*
        Manda mensaje de finalizacion a la consola y cierra el socket correspondiente
    */

    int32_t dummy = DUMMY_VALUE;

    pcb_t * pcb_pointer = obtener_pcb_pointer_desde_pid(pid);

    if (sockets_enviar_dato(pcb_pointer->consola_socket, &dummy, sizeof(int32_t), logger) == false){
	    log_error(logger, "Error al comunicarse con consola pid %d", pid);
    }

    sockets_cerrar(pcb_pointer->consola_socket);

    log_info(logger, "Se cerró la conexion con consola pid %d", pid);
}

void *gestionar_nuevas_consolas(void * arg){
    /*
        Abre un servidor y escucha nuevas conexiones de consolas
    */

    char buffer[MAX_INSTRUCCION_SIZE];
    int socket_consola_actual = 0;
    char instruccion_o_tamanio[12];
    int32_t tamanio_proceso = 0;
    char *lista_instrucciones = NULL;

    sockets_abrir_servidor(PUERTO_ESCUCHA, CONSOLA_BACKLOG, &consolas_socket, logger);

    while(1){
        sockets_esperar_cliente(consolas_socket, &socket_consola_actual, logger);

        do{
            sockets_recibir_string(socket_consola_actual, instruccion_o_tamanio, logger);

            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(socket_consola_actual, buffer, logger);
                agregar_instruccion_a_lista(&lista_instrucciones, buffer);
            }
            else{
                sockets_recibir_dato(socket_consola_actual, &tamanio_proceso, sizeof (int32_t), logger);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

        generar_pcb(lista_instrucciones, tamanio_proceso, socket_consola_actual);
        transicion_new_ready();
        
        lista_instrucciones = NULL;
    }

    return NULL;
}
void probar_conexion_consola(void){
    /*
        Es para testear
    */
    
    sleep(7);

    int i = 0;
    printf("todos_pcb_length: %d\n", todos_pcb_length);
    for (i = 0; i < todos_pcb_length; i++){
        printf("\nLista instrucciones: %s\n", (todos_pcb[i]).lista_instrucciones);
    }

    sleep(60);

    liberar_memoria();
}