#include "kernel.h"

void inicializar_estructuras_memoria(pcb_t* pcb_pointer){
    /*
        Se comunica con memoria cuando un nuevo proceso va a entrar a READY
        para recibir la entrada de tabla de 1er nivel
    */
    
    // TODO: IMPLEMENTAR

    // NOTA: el siguiente codigo es solo de prueba. No es valido
    
    pcb_pointer->tabla_paginas = NULL;
}

void memoria_suspender_proceso(pcb_t* pcb_pointer){

    // TODO: IMPLEMENTAR

    return;
}

void memoria_volver_de_suspendido(pcb_t* pcb_pointer){
    
    // TODO: IMPLEMENTAR
    
    return;
}

void liberar_estructuras_memoria(void){
    //TODO: IMPLEMENTAR
    return;
}