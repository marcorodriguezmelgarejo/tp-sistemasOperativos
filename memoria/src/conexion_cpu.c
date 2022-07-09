#include "memoria.h"

void esperar_conexion_cpu(int socket_escucha){
    log_info(logger, "Esperando conexion de cpu...");
    if(!sockets_esperar_cliente(socket_escucha, &cpu_socket, logger)){
        log_error(logger, "Error al conectar al cpu");
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
    int32_t motivo;

    while(true){
        sockets_recibir_dato(cpu_socket, &motivo, sizeof(motivo), logger);

        pthread_mutex_lock(&mutex_conexiones);

        usleep(milisegundos_a_microsegundos(RETARDO_MEMORIA)); // Retardo para simular un acceso a una memoria

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
                if(!atender_acceso_lectura_espacio_usuario()){
                    log_error(logger, "Error al leer el en el espacio de usuario");
                }
                break;
            case ESCRITURA_EN_ESPACIO_USUARIO:
                if(!atender_acceso_escritura_espacio_usuario()){
                    log_error(logger, "Error al escribir el en el espacio de usuario");
                }
                break;
            default:
                log_error(logger, "Error al recibir el motivo de comunicacion de la CPU");
                break;
        }

        pthread_mutex_unlock(&mutex_conexiones);

    }
    return NULL;
}

// testear
bool atender_acceso_tabla_primer_nivel(void){

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

    if((int)tabla_del_proceso == -1){
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

// testear
bool atender_acceso_tabla_segundo_nivel(void){

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

    if((int)tabla_del_proceso == -1){
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

bool atender_acceso_lectura_espacio_usuario(void){
    // INPUT: MARCO, READ
    // OUTPUT: CONTENIDO MARCO
    int32_t buffer_pid;
    int32_t buffer_num_pagina;
    int32_t buffer_marco;
    int32_t buffer_desplazamiento;
    int32_t valor_leido;

    tabla_primer_nivel* tabla_del_proceso;

    if(!sockets_recibir_dato(cpu_socket, &buffer_pid, sizeof buffer_pid, logger)){
        log_error(logger, "Error al recibir pid");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_num_pagina, sizeof buffer_num_pagina, logger)){
        log_error(logger, "Error al recibir el numero de pagina");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_marco, sizeof buffer_marco, logger)){
        log_error(logger, "Error al recibir el marco");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_desplazamiento, sizeof buffer_desplazamiento, logger)){
        log_error(logger, "Error al recibir el desplazamiento");
        return false;
    }

    tabla_del_proceso = obtener_tabla_con_pid(buffer_pid);
    
    valor_leido = acceder_espacio_usuario_lectura(tabla_del_proceso, buffer_num_pagina, buffer_marco, buffer_desplazamiento);

    if(!sockets_enviar_dato(cpu_socket, &valor_leido, sizeof valor_leido, logger)){
        log_error(logger, "Error al enviar el dato leido");
        return false;
    }
    log_debug(logger, "Dato leido (%d) enviado a cpu", valor_leido);

    return true;
}

bool atender_acceso_escritura_espacio_usuario(void){
    // INPUT: MARCO, WRITE, CONTENT(PARAM)
    // OUTPUT: "OK"
    int32_t buffer_pid;
    int32_t buffer_num_pagina;
    int32_t buffer_marco;
    int32_t buffer_desplazamiento;
    int32_t buffer_valor;

    tabla_primer_nivel* tabla_del_proceso;

    if(!sockets_recibir_dato(cpu_socket, &buffer_pid, sizeof buffer_pid, logger)){
        log_error(logger, "Error al recibir pid");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_num_pagina, sizeof buffer_num_pagina, logger)){
        log_error(logger, "Error al recibir el numero de pagina");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_marco, sizeof buffer_marco, logger)){
        log_error(logger, "Error al recibir el marco");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_desplazamiento, sizeof buffer_desplazamiento, logger)){
        log_error(logger, "Error al recibir el desplazamiento");
        return false;
    }

    if(!sockets_recibir_dato(cpu_socket, &buffer_valor, sizeof buffer_valor, logger)){
        log_error(logger, "Error al recibir el valor para escribir");
        return false;
    }

    tabla_del_proceso = obtener_tabla_con_pid(buffer_pid);
    
    if(!acceder_espacio_usuario_escritura(tabla_del_proceso, buffer_num_pagina, buffer_marco, buffer_desplazamiento, buffer_valor)){
        log_error(logger, "Error al acceder al espacio de usuario para escritura. Marco: %d, desplazamiento: %d", buffer_marco, buffer_desplazamiento);
        return false;
    }
    
    sockets_enviar_string(cpu_socket, "OK", logger);

    return true;
}
