#include "cpu.h"

/* 
Si no hay pcb ejecutando, ho hace nada

Si hay un error enviando el pcb, igual borra el que esta en ejecucion y deja el cpu esperando otro pcb
TODO: (ver con los companeros/profes) capaz no quiero hacer eso, capaz quiero en su lugar:
  - si es por i/o o int que saltee la instruccion? (habria que hacer desalojar() y enviar() por 
    separado y segun el resultado de enviar(), decidir si desalojar o no. Ya no se podria usar
    siempre la misma funcion desalojar_y_devolver())
  - que directamente mate el proceso CPU
  - que haga lo que hace ahora pero vuelva a intentar conectarse con el kernel, para
    que sea posible reiniciar el proceso Kernel? (capaz es demasiado)

*/

bool desalojar_y_devolver_pcb(char * motivo){

    int pid;
    pthread_mutex_lock(&mutex_PCB);
    pid = en_ejecucion.pid;
    pthread_mutex_unlock(&mutex_PCB);
    if (pid == PID_NULO){
        log_error(logger, "Se quiso desalojar el proceso pero no hay proceso en ejecucion");
        return false;
    }
    log_info(logger, "Desalojando el proceso del CPU por %s", motivo);

    if(!devolver_pcb(motivo)){
        desalojar_pcb();
        return false;
    }

    desalojar_pcb();

    if(sem_post(&CPU_vacia) != 0){
        log_error(logger, "Error en el semaforo CPU_vacia");
        exit(ERROR_STATUS);
    }

    return true;
}

void desalojar_pcb(){
    finalizar = true;

    pthread_mutex_lock(&mutex_PCB);
    free(en_ejecucion.lista_instrucciones);
    en_ejecucion.lista_instrucciones = NULL;
    en_ejecucion.pid = PID_NULO;
    pthread_mutex_unlock(&mutex_PCB);
}

bool devolver_pcb(char* motivo){
    if(!sockets_enviar_string(dispatch_socket, motivo, logger)){
        log_error(logger, "No se pudo enviar el motivo de la devolucion del PCB");
        return false;
    }

    if(!sockets_enviar_pcb(dispatch_socket, en_ejecucion, logger)){
        log_error(logger, "No se pudo enviar PCB al Kernel");
        return false;
    }

    return true;
}