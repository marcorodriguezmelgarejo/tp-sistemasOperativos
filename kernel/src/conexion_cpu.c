#include "kernel.h"

void *gestionar_dispatch(void *arg){
    /*
        Escucha a la espera de mensajes del CPU mediante el puerto dispatch y maneja la rutina
        dependiendo del motivo del mensaje de la CPU
    */

    char motivo[5]; // "I/O", "EXIT" o "INT"
    /*
    "I/O" : el proceso llego a instruccion "I/O"
    "EXIT": el proceso llego a instruccion "EXIT"
    "INT" : el kernel previamente mando un mensaje de interrupcion al CPU
    */
    pcb_t pcb_buffer;
    int32_t tiempo_bloqueo = 0;

    while(1){
        //recibe el motivo del mensaje
        sockets_recibir_string(dispatch_socket, motivo, logger);
        
        //recibe el pcb del proceso en ejecucion
        pcb_buffer.lista_instrucciones = NULL;
        sockets_recibir_pcb(dispatch_socket, &pcb_buffer, logger);
        
        actualizar_program_counter_en_ejecucion(pcb_buffer.program_counter); 

        free(pcb_buffer.lista_instrucciones); //debido a que sockets_recibir_pcb() reservo memoria 

        if (strcmp(motivo, "I/O") == 0){
            //recibo el tiempo de bloqueo
            sockets_recibir_dato(dispatch_socket, &tiempo_bloqueo, sizeof tiempo_bloqueo, logger);
            gestionar_proceso_a_io();
        }
        else if (strcmp(motivo, "EXIT") == 0){
            transicion_ejec_exit();
        }
        else if (strcmp(motivo, "INT") == 0){
            transicion_ejec_ready();
        }
        
    }
}

void enviar_pcb_cpu(pcb_t* pcb_pointer){
    if (sockets_enviar_pcb(dispatch_socket, *pcb_pointer, logger) == false){
        log_error(logger, "Error al enviar pcb al cpu");
    }
}

void gestionar_proceso_a_io(void){
    /*
        Cuando un proceso se va a I/O
    */ 

    list_add(lista_bloqueado, &en_ejecucion);

    en_ejecucion = NULL;
}

void gestionar_interrupcion_kernel(void){
    /*
        Cuando se recibe el pcb luego de que el kernel haya mandado una interrupcion al cpu
    */
    list_add(lista_ready, &en_ejecucion);
    en_ejecucion = NULL;
}

void conectar_puerto_dispatch(void){
    
    if (sockets_conectar_como_cliente(IP_CPU, PUERTO_CPU_DISPATCH, &dispatch_socket, logger) == false){
        log_error(logger, "Error al conectar con puerto dispatch. Finalizando...");
        exit(ERROR_STATUS);
    }
    log_debug(logger, "Conectado al puerto dispatch del CPU");
}

void conectar_puerto_interrupt(void){

    if (sockets_conectar_como_cliente(IP_CPU, PUERTO_CPU_INTERRUPT, &interrupt_socket, logger) == false){
        log_error(logger, "Error al conectar con puerto interrupt. Finalizando...");
        exit(ERROR_STATUS);
    }
    log_debug(logger, "Conectado al puerto interrupt del CPU");
}

void enviar_interrupcion_cpu(void){

    int32_t dato = DUMMY_VALUE;

    if (sockets_enviar_dato(interrupt_socket, &dato, sizeof dato, logger) == false){
        log_error(logger, "Error al enviar interrupcion al CPU");
    }
}