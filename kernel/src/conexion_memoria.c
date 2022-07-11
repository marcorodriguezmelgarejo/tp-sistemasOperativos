#include "kernel.h"

void inicializar_estructuras_memoria(pcb_t* pcb_pointer){
    /*
        Se comunica con memoria cuando un nuevo proceso va a entrar a READY
        para recibir la entrada de tabla de 1er nivel
    */
    int32_t tamanio_proceso = pcb_pointer->tamanio;

    pthread_mutex_lock(&mutex_cola_instrucciones_memoria);
    solicitar_operacion_a_memoria(pcb_pointer->pid, INICIALIZAR_PROCESO);

    if(!sockets_enviar_dato(memoria_socket, &tamanio_proceso, sizeof tamanio_proceso, logger)){
        log_error(logger, "Error al enviar el tamanio del proceso");
    }

    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al inicializar estructuras de pid %d", pcb_pointer->pid);
    }

    pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);
}

void memoria_suspender_proceso(pcb_t* pcb_pointer){
    pthread_mutex_lock(&mutex_cola_instrucciones_memoria);

    solicitar_operacion_a_memoria(pcb_pointer->pid, SUSPENDER_PROCESO);
    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al suspender el proceso pid %d", pcb_pointer->pid);
    }
    pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);

}

void memoria_finalizar_proceso(pcb_t* pcb_pointer){
    pthread_mutex_lock(&mutex_cola_instrucciones_memoria);

    solicitar_operacion_a_memoria(pcb_pointer->pid, FINALIZAR_PROCESO);
    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al liberar la memoria de pid %d", pcb_pointer->pid);
    }
    pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);

}

void enviar_fin_memoria(void){
    
    int32_t motivo = FIN_MEMORIA;
    pthread_mutex_lock(&mutex_cola_instrucciones_memoria);

    if(!sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)){
        log_error(logger, "Error al enviar el motivo de la comunicacion a memoria");
    }
    pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);

}

void conectar_puerto_memoria(void){
    if(!sockets_conectar_como_cliente(IP_MEMORIA, PUERTO_MEMORIA, &memoria_socket, logger)){
        log_error(logger, "Error al conectar con memoria");
        exit(ERROR_STATUS);
    }
    log_debug(logger, "Conectado a la memoria");
}

// la usan las tres funciones de comunicacion con memoria
void solicitar_operacion_a_memoria(int32_t pid, int32_t motivo){

    if(!sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)){
        log_error(logger, "Error al enviar el motivo de la comunicacion a memoria");
    }
    
    if(!sockets_enviar_dato(memoria_socket, &pid, sizeof pid, logger)){
        log_error(logger, "Error al enviar el pid del proceso");
    }

}
// la usan las tres funciones de comunicacion con memoria
bool recibir_respuesta_memoria(void){
    char respuesta[5];

    if(!sockets_recibir_string(memoria_socket, respuesta, logger)){
        log_error(logger, "Error al recibir la confirmacion de memoria");
        return false;
    }

    if(strcmp("OK", respuesta) != 0){
        log_error(logger, "La memoria arrojo un codigo de error");
        return false;
    }

    return true;
}