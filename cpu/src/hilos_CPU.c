#include "cpu.h"

/*
        Uso semaforos e hilos para poder guardar el mensaje de interrupcion del kernel y el PCB nuevo en cualquier
        momento de la ejecucion del ciclo de instruccion. La atiendo y cambio el PCB solamente en la fase chequear_interrupcion()
        (no se puede cambiar el PCB en ejecucion en el medio de una operacion).

*/
bool inicializar_semaforos(){
    if(sem_init(&PCB_en_CPU, 0, 0) != 0){
        log_error(logger, "Error al inicializar semaforo PCB_en_CPU");
        return false;
    }    
    if(sem_init(&CPU_vacia, 0, 1) != 0){
        log_error(logger, "Error al inicializar semaforo CPU_vacia");
        return false;
    }        
    if(pthread_mutex_init(&mutex_PCB, NULL) != 0){
        log_error(logger, "Error al inicializar mutex pcb");
        return false;
    }    
    if(pthread_mutex_init(&mutex_interrupcion, NULL) != 0){
        log_error(logger, "Error al inicializar mutex flag interrupcion");
        return false;
    }
    return true;
}

// hilo
void esperar_pcb(){
    pcb_t pcb_buffer;
    pcb_buffer.lista_instrucciones = NULL;

    while(true){
        // recibe el pcb en cualquier punto del ciclo de intruccion
        sockets_recibir_pcb(dispatch_socket, &pcb_buffer, logger);
        log_info(logger, "PCB recibido. Se introducira cuando la CPU este vacia.");

        // solamente introduce el PCB despues de que el anterior haya sido desalojado
        sem_wait(&CPU_vacia);

        pthread_mutex_lock(&mutex_PCB);
        en_ejecucion = pcb_buffer;
        en_ejecucion.lista_instrucciones = malloc(strlen(pcb_buffer.lista_instrucciones)+1);
        strcpy(en_ejecucion.lista_instrucciones, pcb_buffer.lista_instrucciones);
        pthread_mutex_unlock(&mutex_PCB);

        free(pcb_buffer.lista_instrucciones);
        pcb_buffer.lista_instrucciones = NULL;

        log_debug(logger, "PCB introducido a ejecuccion, pid: %d ", en_ejecucion.pid);

        sem_post(&PCB_en_CPU);
    }
}

// hilo
void esperar_interrupcion(){
    int32_t buffer = 0;

    while(true){
        sockets_recibir_dato(interrupt_socket, &buffer, sizeof buffer, logger);

        log_info(logger, "Se recibio interrupcion del kernel");

        pthread_mutex_lock(&mutex_interrupcion);
        interrupcion = true;
        pthread_mutex_unlock(&mutex_interrupcion);

        log_info(logger, "Se marco el flag interrupcion como True");
    }
}

// un hilo
void ciclo_instruccion(){
    char string_instruccion[MAX_INSTRUCCION_SIZE];
    instruccion_t instruccion;

    while(true){
        log_info(logger, "Esperando PCB...");
        sem_wait(&PCB_en_CPU);

        loguear_PC();

        while(true){
            fetch(string_instruccion);
            instruccion = decode(string_instruccion);
            if(execute(instruccion)){
                log_info(logger, "Operacion ejecutada con exito");
            }
            else{
                log_error(logger, "Error en la ejecucion de la operacion");
            }
            chequear_interrupcion();
            if(finalizar){ // lo setean en true las operaciones exit, i_o y la funcion chequear_interrupcion() en el caso de haber una interrupcion
                finalizar = false;
                break;
            }
        }
    }
}