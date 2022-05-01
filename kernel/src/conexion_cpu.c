#include "kernel.h"

void *gestionar_dispatch(void *arg){
    /*
        Abre un servidor y escucha a la espera de mensajes del CPU mediante el puerto dispatch
    */

    int temp_socket = 0;
    char motivo[5]; // "I/O", "EXIT" o "INT"
    pcb_t pcb_buffer;

    sockets_abrir_servidor(PUERTO_CPU_DISPATCH, CONSOLA_BACKLOG, &temp_socket, logger);

    sockets_esperar_cliente(temp_socket, &dispatch_socket, logger);

    sockets_cerrar(temp_socket); //cierro el servidor porque no espero mas clientes

    while(1){

        //recibe la razon del mensaje (si el proceso se fue a i/o, si llego a instruccion "EXIT" o si fue por interrupcion del kernel)
        sockets_recibir_string(dispatch_socket, motivo, logger);
        sockets_recibir_pcb(dispatch_socket, &pcb_buffer, logger);

        actualizar_pcb(pcb_buffer);

        if (strcmp(motivo, "I/O") == 0){
            gestionar_proceso_a_io();
        }
        else if (strcmp(motivo, "EXIT") == 0){
            planificador_largo_plazo_exit();
        }
        else if (strcmp(motivo, "INT") == 0){
            gestionar_interrupcion_kernel();
        }
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

void conectar_puerto_interrupt(void){

    if (sockets_conectar_como_cliente(IP_CPU, PUERTO_CPU_INTERRUPT, &interrupt_socket, logger) == false){
        log_error(logger, "Error al conectar con puerto interrupt. Finalizando...");
        exit(ERROR_STATUS);
    }
}

void enviar_interrupcion_cpu(void){

    int32_t dato = DUMMY_VALUE;

    if (sockets_enviar_dato(interrupt_socket, &dato, sizeof dato, logger) == false){
        log_error(logger, "Error al enviar interrupcion al CPU");
    }
}