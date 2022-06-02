#include "cpu.h"
#include "protocolos.h"



void *conectar_dispatch(void *arg){

    int temp_socket = 0;

    sockets_abrir_servidor(PUERTO_ESCUCHA_DISPATCH, SOMAXCONN, &temp_socket, logger);
    
    sockets_esperar_cliente(temp_socket, &dispatch_socket, logger);
    
    sockets_cerrar(temp_socket); //cierro el servidor porque no espero mas clientes

    return NULL;
}

void *conectar_interrupt(void *arg){

    int temp_socket_1 = 0;

    sockets_abrir_servidor(PUERTO_ESCUCHA_INTERRUPT, SOMAXCONN, &temp_socket_1, logger);
    
    sockets_esperar_cliente(temp_socket_1, &interrupt_socket, logger);
    
    sockets_cerrar(temp_socket_1); //cierro el servidor porque no espero mas clientes

    return NULL;
}