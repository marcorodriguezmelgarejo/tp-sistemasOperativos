#include "memoria.h"


void esperar_conexion_kernel(int socket_escucha){
    log_info(logger, "Esperando conexion de kernel...");
    if(!sockets_esperar_cliente(socket_escucha, &kernel_socket, logger)){
        log_error(logger, "Error al conectar al kernel");
        exit(ERROR_STATUS);
    }
    return;
}


void* hilo_escuchar_kernel(void * arg){
    int32_t motivo;
    int32_t pid;

    while(true){

        if(!sockets_recibir_dato(kernel_socket, &motivo, sizeof motivo, logger)){
            log_error(logger, "Error al recibir motivo de comunicacion por parte de kernel");
            continue;
        }
        
        if(!sockets_recibir_dato(kernel_socket, &pid, sizeof pid, logger)){
            log_error(logger, "Error al recibir pid de kernel");
            continue;
        }
        
        pthread_mutex_lock(&mutex_conexiones);

        switch(motivo){
            case INICIALIZAR_PROCESO:
                atender_inicializacion_proceso(pid);
                break;
            case SUSPENDER_PROCESO:
                atender_suspension_proceso(pid);
                break;
            case FINALIZAR_PROCESO:
                atender_finalizacion_proceso(pid);
                break;
            case FIN_MEMORIA:
                atender_fin_memoria();
                return NULL;
                break;
            default:
                log_error(logger, "Motivo invalido");
        }

        pthread_mutex_unlock(&mutex_conexiones);
    }

    return NULL;
}

// testear
void atender_finalizacion_proceso(int32_t pid){
    tabla_primer_nivel* tabla_primer_nivel_pointer = obtener_tabla_con_pid(pid);
    finalizar_proceso(tabla_primer_nivel_pointer);

    if(!sockets_enviar_string(kernel_socket, "OK", logger)){
        log_error(logger, "Error al enviar mensaje de confirmacion a memoria");
    }
}

// testear
void atender_suspension_proceso(int32_t pid){
    tabla_primer_nivel* tabla_primer_nivel_pointer = obtener_tabla_con_pid(pid);
    suspender_proceso(tabla_primer_nivel_pointer);

    if(!sockets_enviar_string(kernel_socket, "OK", logger)){
        log_error(logger, "Error al enviar mensaje de confirmacion a memoria");
    }
}

// testear
void atender_inicializacion_proceso(int32_t pid){
    int32_t tamanio_proceso;

    if(!sockets_recibir_dato(kernel_socket, &tamanio_proceso, sizeof tamanio_proceso, logger)){
        log_error(logger, "Error al recibir tamanio_proceso de kernel");
    }

    if(inicializar_proceso(pid, tamanio_proceso) == NULL){
        log_error(logger, "No se pudo inicializar el proceso pid %d", pid);
        sockets_enviar_string(kernel_socket, "ERROR", logger);
        return;
    }

    if(!sockets_enviar_string(kernel_socket, "OK", logger)){
        log_error(logger, "Error al enviar mensaje de confirmacion a memoria");
    }
}

void atender_fin_memoria(void){
    kill(getpid(), SIGINT);
}