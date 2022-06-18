#include "cpu.h"
#include "protocolos.h"

bool conectar_con_kernel(){
    pthread_t hilo_conectar_dispatch, hilo_conectar_interrupt;

    if(pthread_create(&hilo_conectar_dispatch, NULL, (void*) conectar_dispatch, NULL) != 0){
        log_error(logger, "No se pudo crear el hilo para conectar con dispatch");
        exit(ERROR_STATUS);
    }

    if(pthread_create(&hilo_conectar_interrupt, NULL, (void*) conectar_interrupt, NULL) != 0){
        log_error(logger, "No se pudo crear el hilo para conectar con interrupt");
        exit(ERROR_STATUS);
    }

    pthread_join(hilo_conectar_dispatch, NULL);
    pthread_join(hilo_conectar_interrupt, NULL);

    return true;
}

void conectar_con_memoria(){
    if(!sockets_conectar_como_cliente(IP_MEMORIA, PUERTO_MEMORIA, &memoria_socket, logger)){
        log_error(logger, "Error al conectarse con memoria");
        exit(ERROR_STATUS);
    }

    if(!handshake_memoria()){
        log_error(logger, "Error en el handsake con memoria, finalizando ejecucion...");
        exit(ERROR_STATUS);
    }
    
    log_debug(logger, "Memoria conectada!");
}

bool handshake_memoria(){
    char string_buffer[100];
    int32_t int_buffer;

    sockets_recibir_string(memoria_socket, string_buffer, logger);
    if(strcmp(string_buffer, "cantidad de entradas tabla de paginas") != 0){
        return false;
    }

    if(!sockets_recibir_dato(memoria_socket, (void*) &int_buffer, sizeof int_buffer, logger)){
        return false;
    }
    entradas_tabla_paginas = int_buffer;

    sockets_recibir_string(memoria_socket, string_buffer, logger);
    if(strcmp(string_buffer, "tamanio pagina") != 0){
        return false;
    }

    if(!sockets_recibir_dato(memoria_socket, (void*) &int_buffer, sizeof int_buffer, logger)){
        return false;
    }
    tamanio_pagina = int_buffer;

    log_info(logger, "Cantidad de entradas tabla de pagina: %d", entradas_tabla_paginas);
    log_info(logger, "Tamanio pagina: %d", tamanio_pagina);

    return true;
}

bool conectar_dispatch(){

    int temp_socket = 0;

    if(!sockets_abrir_servidor(PUERTO_ESCUCHA_DISPATCH, SOMAXCONN, &temp_socket, logger)){
        log_error(logger, "Error al abrir servidor temporal para conectar a dispatch");
        exit(ERROR_STATUS);
    }
    
    log_info(logger, "Esperando conexion de kernel en dispatch...");
    if(!sockets_esperar_cliente(temp_socket, &dispatch_socket, logger)){
        log_error(logger, "Error al conectar al puerto de dispatch de kernel");
        exit(ERROR_STATUS);
    }


    sockets_cerrar(temp_socket); //cierro el servidor porque no espero mas clientes

    log_debug(logger, "Dispatch conectado!");
    return true;
}

bool conectar_interrupt(){

    int temp_socket_1 = 0;

    if(!sockets_abrir_servidor(PUERTO_ESCUCHA_INTERRUPT, SOMAXCONN, &temp_socket_1, logger)){
        log_error(logger, "Error al abrir servidor temporal para conectar a interrupt");
        exit(ERROR_STATUS);
    }
    
    log_info(logger, "Esperando conexion de kernel en interrupt...");
    if(!sockets_esperar_cliente(temp_socket_1, &interrupt_socket, logger)){
        log_error(logger, "Error al conectar al puerto de interrupt de kernel");
        exit(ERROR_STATUS);
    }
    
    sockets_cerrar(temp_socket_1); //cierro el servidor porque no espero mas clientes

    log_debug(logger, "Interrupt conectado!");
    return true;
}

void matar_kernel(){
    if(!sockets_enviar_string(dispatch_socket, "FIN_CPU", logger)){
        log_error(logger, "No se pudo enviar el mensaje de finalizacion al kernel");
    }
}