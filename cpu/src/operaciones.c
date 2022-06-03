#include "cpu.h"

bool no_op(){
    usleep(RETARDO_NOOP * 1000);
    return true;
}

// TODO: EXIT
bool salir(){ // es la operacion exit
    if(!desalojar_y_devolver_pcb("EXIT")){
        log_error(logger, "Error en la finalizacion del proceso, no se pudo devolver el PCB a Kernel");
    }
    finalizar = true;
    return true;
}

// TODO: I_O