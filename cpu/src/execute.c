#include "cpu.h"

bool execute(instruccion_t instruccion){
    bool respuesta_operacion;

    switch(instruccion.operacion){
        case NO_OP:
            respuesta_operacion = no_op();
            break;
        case INVALIDA:
            log_error(logger, "Error al ejecutar, operacion invalida");
            respuesta_operacion = false;
            break;
        default:
            log_error(logger, "Error al ejecutar, codigo de operacion desconocido");
            respuesta_operacion = false;
            break;
    }

    return respuesta_operacion;
}