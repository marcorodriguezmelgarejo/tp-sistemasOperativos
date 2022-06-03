#include "cpu.h"
#include "protocolos.h"

bool conectar_con_kernel(){
    pthread_t hilo_conectar_dispatch, hilo_conectar_interrupt;


    if(pthread_create(&hilo_conectar_dispatch, NULL, (void*) conectar_dispatch, NULL) == 0){
        log_debug(logger, "Hilo para conectar con dispatch creado");
    }else{
        log_error(logger, "No se pudo crear el hilo para conectar con dispatch");
        exit(ERROR_STATUS);
    }

    if(pthread_create(&hilo_conectar_interrupt, NULL, (void*) conectar_interrupt, NULL) == 0){
        log_debug(logger, "Hilo para conectar con interrupt creado");
    }else{
        log_error(logger, "No se pudo crear el hilo para conectar con interrupt");
        exit(ERROR_STATUS);
    }

    pthread_join(hilo_conectar_dispatch, NULL);
    pthread_join(hilo_conectar_interrupt, NULL);

    return true;
}

bool conectar_dispatch(){

    int temp_socket = 0;

    log_info(logger, "Abriendo servidor p dispatch...");
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

    log_info(logger, "Abriendo servidor p interrupt...");
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