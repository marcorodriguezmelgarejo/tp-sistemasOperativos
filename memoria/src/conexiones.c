#include "memoria.h"

void conectar_cpu_y_kernel(void){
    int server_socket;

    if(!sockets_abrir_servidor(PUERTO_ESCUCHA, 5, &server_socket, logger)){
        log_error(logger, "Error al abrir servidor para esperar conexiones, finalizando ejecucion...");
        exit(ERROR_STATUS);
    }

  //  esperar_conexion_kernel(server_socket);
    esperar_conexion_cpu(server_socket);
    if(!handshake_cpu()){
        log_error(logger, "Error en el handshake con el CPU");
    }

    sockets_cerrar(server_socket);
    log_debug(logger, "Conexiones creadas con exito");
}

void esperar_conexion_cpu(int socket_escucha){
    log_info(logger, "Esperando conexion de cpu...");
    if(!sockets_esperar_cliente(socket_escucha, &cpu_socket, logger)){
        log_error(logger, "Error al conectar al cpu");
        exit(ERROR_STATUS);
    }
    return;
}

void esperar_conexion_kernel(int socket_escucha){
    log_info(logger, "Esperando conexion de kernel...");
    if(!sockets_esperar_cliente(socket_escucha, &kernel_socket, logger)){
        log_error(logger, "Error al conectar al kernel");
        exit(ERROR_STATUS);
    }
    return;
}

bool handshake_cpu(){
    return 
    sockets_enviar_string(cpu_socket, "cantidad de entradas tabla de paginas", logger)
    &&
    sockets_enviar_dato(cpu_socket, (void*) &ENTRADAS_POR_TABLA, sizeof ENTRADAS_POR_TABLA, logger)
    &&
    sockets_enviar_string(cpu_socket, "tamanio pagina", logger)
    &&
    sockets_enviar_dato(cpu_socket, (void*) &TAM_PAGINA, sizeof TAM_PAGINA, logger);
}

void* hilo_escuchar_cpu(void * arg){
    //TODO: IMPLEMENTAR
    // Esperar mensaje de CPU
    // Decodear mensaje de CPU
    // SWITCH CASES -> Interpretar mensaje
        // ID Tabla 1er nivel
            // INPUT: ID PROCESO, PAGINA DE PROCESO
            // OUTPUT: NRO DE ENTRADA DE TABLA DE 2DO NIVEL
        // ID Tabla de 2do nivel
            // INPUT: ID PROCESO, ID PAG 2DO NIVEL, PAGINA
            // OUTPUT: MARCO
        // Read
            // INPUT: MARCO, READ
            // OUTPUT: CONTENIDO MARCO
        // Write
            // INPUT: MARCO, WRITE, CONTENT(PARAM)
            // OUTPUT: "OK"
    return NULL;
}

void* hilo_escuchar_kernel(void * arg){
    //TODO: IMPLEMENTAR
    // Esperar mensaje de Kernel
    // Decodear mensaje de Kernel
    // SWITCH CASES -> Interpretar mensaje
        // Crear proceso
            // INPUT: ID PROCESO, TAMAÑO PROCESO
            // Call TRASLADAR_PROCESO_A_MEMORIA
            // OUTPUT: "OK"
        // Suspender proceso
            // INPUT: ID PROCESO
            // Call TRASLADAR_PROCESO_A_DISCO
            // OUTPUT: "OK"

    return NULL;
}