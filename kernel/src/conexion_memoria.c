#include "kernel.h"

void * hilo_memoria(void * arg){
    /*
        Hilo que desencola de la cola_instrucciones_memoria y ejecuta la instruccion
        comunicandose con memoria.
    */

    instruccion_memoria* instruccion_pointer;

    while(1){
        sem_wait(&contador_cola_instrucciones_memoria);

        pthread_mutex_lock(&mutex_cola_instrucciones_memoria);
        instruccion_pointer = queue_pop(cola_instrucciones_memoria);
        pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);

        switch(instruccion_pointer->codigo_operacion){
            case INICIALIZAR_PROCESO:
                inicializar_estructuras_memoria(instruccion_pointer->pcb_pointer);
                log_debug(logger, "MEMORIA: INICIALIZAR ESTRUCTURAS (PID = %d)", instruccion_pointer->pcb_pointer->pid);
                break;
            case SUSPENDER_PROCESO:
                memoria_suspender_proceso(instruccion_pointer->pcb_pointer);
                log_debug(logger, "MEMORIA: SUSPENDER PROCESO (PID = %d)", instruccion_pointer->pcb_pointer->pid);
                break;
            case FINALIZAR_PROCESO:
                memoria_finalizar_proceso(instruccion_pointer->pcb_pointer);
                log_debug(logger, "MEMORIA: FINALIZAR PROCESO (PID = %d)", instruccion_pointer->pcb_pointer->pid);
                break;
            default:
                log_error(logger, "Error en hilo_memoria(): el codigo de operacion no existe");
                break;
            
        }

        sem_post(instruccion_pointer->semaforo_pointer);

    }

    return NULL;
}

void enviar_instruccion_memoria(pcb_t* pcb_pointer, int32_t codigo_operacion){
    /*
        Encola la instruccion y espera a su finalizacion.
    */

    instruccion_memoria instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.pcb_pointer = pcb_pointer;
    instruccion.codigo_operacion = codigo_operacion;
    instruccion.semaforo_pointer = &semaforo;

    pthread_mutex_lock(&mutex_cola_instrucciones_memoria);
    queue_push(cola_instrucciones_memoria, &instruccion);
    pthread_mutex_unlock(&mutex_cola_instrucciones_memoria);

    sem_post(&contador_cola_instrucciones_memoria);

    sem_wait(&semaforo);

    sem_destroy(&semaforo);
}

void inicializar_estructuras_memoria(pcb_t* pcb_pointer){
    /*
        Se comunica con memoria cuando un nuevo proceso va a entrar a READY
        para recibir la entrada de tabla de 1er nivel
    */
    int32_t tamanio_proceso = pcb_pointer->tamanio;

    solicitar_operacion_a_memoria(pcb_pointer->pid, INICIALIZAR_PROCESO);

    if(!sockets_enviar_dato(memoria_socket, &tamanio_proceso, sizeof tamanio_proceso, logger)){
        log_error(logger, "Error al enviar el tamanio del proceso");
    }

    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al inicializar estructuras de pid %d", pcb_pointer->pid);
    }

}

void memoria_suspender_proceso(pcb_t* pcb_pointer){
    solicitar_operacion_a_memoria(pcb_pointer->pid, SUSPENDER_PROCESO);
    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al suspender el proceso pid %d", pcb_pointer->pid);
    }
}

void memoria_finalizar_proceso(pcb_t* pcb_pointer){
    solicitar_operacion_a_memoria(pcb_pointer->pid, FINALIZAR_PROCESO);
    if(!recibir_respuesta_memoria()){
        log_error(logger, "Error en memoria al liberar la memoria de pid %d", pcb_pointer->pid);
    }
}

void enviar_fin_memoria(void){
    
    int32_t motivo = FIN_MEMORIA;

    if(!sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)){
        log_error(logger, "Error al enviar el motivo de la comunicacion a memoria");
    }
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