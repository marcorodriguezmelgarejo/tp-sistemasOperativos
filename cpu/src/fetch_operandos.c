#include "cpu.h"

bool fetch_operandos(instruccion_t *instruccion){
    int32_t buffer;

    if(instruccion->operacion == COPY){

        if(!leer_dir_logica(instruccion->dir_origen, &buffer)){
            log_error(logger, "No se pudo obtener el operando de memoria en la direccion logica %s. Marcando operacion como invalida.", instruccion-> dir_origen);
            instruccion->operacion = INVALIDA;
            return false;        
        }
        
        instruccion->valor = buffer;
        log_info(logger, "Operando obtenido en memoria: %d", instruccion->valor);
        return true;
    }
}