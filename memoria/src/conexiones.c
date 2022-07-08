#include "memoria.h"

// Testeado
void conectar_cpu_y_kernel(void){
    int server_socket;

    if(!sockets_abrir_servidor(PUERTO_ESCUCHA, 5, &server_socket, logger)){
        log_error(logger, "Error al abrir servidor para esperar conexiones, finalizando ejecucion...");
        exit(ERROR_STATUS);
    }

    esperar_conexion_cpu(server_socket);
    if(!handshake_cpu()){
        log_error(logger, "Error en el handshake con el CPU");
    }
    esperar_conexion_kernel(server_socket);

    sockets_cerrar(server_socket);
    log_debug(logger, "Conexiones creadas con exito");
    log_info(logger, "------------------------------------");
}


