#include "cpu.h"

bool no_op(){
    usleep(RETARDO_NOOP * 1000);
    return true;
}

bool salir(){ // es la operacion exit
    if(!desalojar_y_devolver_pcb("EXIT")){
        log_error(logger, "Error, no se pudo devolver el PCB a Kernel");
        return false;
    }
    return true;
}

bool i_o(int32_t tiempo_bloqueo){
    if(!desalojar_y_devolver_pcb("I/O")){
        log_error(logger, "Error, no se pudo devolver el PCB a Kernel");
        return false;
    }
    if(!sockets_enviar_dato(dispatch_socket, &tiempo_bloqueo, sizeof tiempo_bloqueo, logger)){
        log_error(logger, "Error, no se pudo enviar el tiempo de bloqueo a Kernel");
        return false;
    }

    return true;
}