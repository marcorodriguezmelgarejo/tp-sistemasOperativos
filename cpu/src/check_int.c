#include "cpu.h"

void chequear_interrupcion(){
    if(interrupcion){
        desalojar_y_devolver_pcb("INT");

        pthread_mutex_lock(&mutex_interrupcion);
        interrupcion = false;
        pthread_mutex_unlock(&mutex_interrupcion);
    }
}

void desalojar_y_devolver_pcb(char * motivo){
    int pid;

    pthread_mutex_lock(&mutex_PCB);
    pid = en_ejecucion.pid;
    pthread_mutex_unlock(&mutex_PCB);

    if (pid == PID_NULO){
        log_error(logger, "Se quiso desalojar el proceso pero no hay proceso en ejecucion");
        return;
    }

    log_info(logger, "Desalojando el proceso del CPU por ", motivo);

    if(!sockets_enviar_string(dispatch_socket, motivo, logger)){
        log_error(logger, "No se pudo enviar el motivo de la interrupcion");
        return;
    }

    if(!sockets_enviar_pcb(dispatch_socket, en_ejecucion, logger)){
        log_error(logger, "No se pudo enviar PCB al Kernel");
        return;
    }

    log_info(logger, "PCB devuelto al Kernel");

    pthread_mutex_lock(&mutex_PCB);
    free(en_ejecucion.lista_instrucciones);
    en_ejecucion.lista_instrucciones = NULL;
    en_ejecucion.pid = PID_NULO;
    pthread_mutex_unlock(&mutex_PCB);

    sem_post(&CPU_vacia);

    log_info(logger, "CPU libre");
}