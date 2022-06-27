#include "memoria.h"

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


bool atender_acceso_tabla_primer_nivel(){

    // ID Tabla 1er nivel
        // INPUT: ID PROCESO, PAGINA DE PROCESO
        // OUTPUT: NRO DE ENTRADA DE TABLA DE 2DO NIVEL

    int32_t buffer_pid;
    int32_t buffer_num_pagina;

    tabla_primer_nivel* tabla_del_proceso;
    int32_t numero_tabla_segundo_nivel;


    if(!sockets_recibir_dato(cpu_socket, &buffer_pid, sizeof buffer_pid, logger)){
        log_error(logger, "Error al recibir pid");
        return false;
    }
    if(!sockets_recibir_dato(cpu_socket, &buffer_num_pagina, sizeof buffer_num_pagina, logger)){
        log_error(logger, "Error al recibir el numero de pagina");
        return false;
    }

    tabla_del_proceso = obtener_tabla_con_pid(buffer_pid);

    if(tabla_del_proceso == -1){
        return false;
    }

    numero_tabla_segundo_nivel = acceder_tabla_primer_nivel(tabla_del_proceso, buffer_num_pagina);

    if(numero_tabla_segundo_nivel == -1){
        return false;
    }

    if(!sockets_enviar_dato(cpu_socket, &numero_tabla_segundo_nivel, sizeof numero_tabla_segundo_nivel, logger)){
        log_error(logger, "Error al enviar el numero de tabla de segundo nivel");
        return false;
    }

    return true;
}

bool atender_acceso_tabla_segundo_nivel(){

    // ID Tabla de 2do nivel
        // INPUT: ID PROCESO, ID PAG 2DO NIVEL, PAGINA
        // OUTPUT: MARCO

    int32_t buffer_pid;
    int32_t buffer_num_pagina;
    int32_t buffer_numero_tabla_segundo_nivel;

    tabla_primer_nivel* tabla_del_proceso;
    int32_t numero_marco;


    if(!sockets_recibir_dato(cpu_socket, &buffer_pid, sizeof buffer_pid, logger)){
        log_error(logger, "Error al recibir pid");
        return false;
    }
    if(!sockets_recibir_dato(cpu_socket, &buffer_numero_tabla_segundo_nivel, sizeof buffer_numero_tabla_segundo_nivel, logger)){
        log_error(logger, "Error al recibir el numero de tabla de segundo nivel");
        return false;
    }
    if(!sockets_recibir_dato(cpu_socket, &buffer_num_pagina, sizeof buffer_num_pagina, logger)){
        log_error(logger, "Error al recibir el numero de pagina");
        return false;
    }

    tabla_del_proceso = obtener_tabla_con_pid(buffer_pid);

    if(tabla_del_proceso == -1){
        return false;
    }

    numero_marco = acceder_tabla_segundo_nivel(tabla_del_proceso, buffer_numero_tabla_segundo_nivel, buffer_num_pagina);

    if(numero_marco == -1){
        return false;
    }

    if(!sockets_enviar_dato(cpu_socket, &numero_marco, sizeof numero_marco, logger)){
        log_error(logger, "Error al enviar el numero de marco");
        return false;
    }

    return true;
}

void* hilo_escuchar_cpu(void * arg){
    int32_t motivo;

    while(true){
        sockets_recibir_dato(cpu_socket, &motivo, sizeof(motivo), logger);

        switch(motivo){
            case ACCESO_TABLA_PRIMER_NIVEL:
                if(!atender_acceso_tabla_primer_nivel()){
                    log_error(logger, "Error al atender la peticion de acceso a tabla de primer nivel");
                }
                break;

            case ACCESO_TABLA_SEGUNDO_NIVEL:
                if(!atender_acceso_tabla_segundo_nivel()){
                    log_error(logger, "Error al atender la peticion de acceso a tabla de segundo nivel");
                }
                break;
            case LECTURA_EN_ESPACIO_USUARIO:

                break;
            case ESCRITURA_EN_ESPACIO_USUARIO:

                break;
            default:
                log_error(logger, "Error al recibir el motivo de comunicacion de la CPU");
                break;
        }


    }




    // TODO: IMPLEMENTAR
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
    // TODO: IMPLEMENTAR
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