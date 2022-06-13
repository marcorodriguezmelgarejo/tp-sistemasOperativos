#include "memoria.h"

void conectar_cpu_y_kernel(void){
    int server_socket;

    sockets_abrir_servidor(PUERTO_ESCUCHA, 5, &server_socket, logger);

    sockets_esperar_cliente(server_socket, &kernel_socket, logger);
    sockets_esperar_cliente(server_socket, &cpu_socket, logger);
}

void* hilo_escuchar_cpu(void * arg){
    //TODO: IMPLEMENTAR
    return NULL;
}

void* hilo_escuchar_kernel(void * arg){
    //TODO: IMPLEMENTAR
    return NULL;
}