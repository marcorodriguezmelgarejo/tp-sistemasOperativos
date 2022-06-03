#include "cpu.h"

bool execute(instruccion_t instruccion){
    bool respuesta_operacion;

    switch(instruccion.operacion){
        case NO_OP:
            respuesta_operacion = no_op();
            break;
        case I_O:
            respuesta_operacion = i_o(instruccion.tiempo_bloqueo);
            break;
        case EXIT:
            respuesta_operacion = salir();
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