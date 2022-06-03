#include "cpu.h"

bool chequear_interrupcion(){
    if(interrupcion){
        if(!desalojar_y_devolver_pcb("INT")){
            log_error(logger, "Habia una interrupcion pero no se pudo desalojar el proceso de la CPU con exito");
            return false;
        }

        pthread_mutex_lock(&mutex_interrupcion);
        interrupcion = false;
        pthread_mutex_unlock(&mutex_interrupcion);

        finalizar = true;
    }
    return true;
}