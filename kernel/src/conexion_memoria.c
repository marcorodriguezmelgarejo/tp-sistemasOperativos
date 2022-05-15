#include "kernel.h"

void inicializar_estructuras_memoria(pcb_t* pcb_pointer){
    /*
        Se comunica con memoria cuando un nuevo proceso va a entrar a READY
        para recibir la entrada de tabla de 1er nivel
    */
    
    // TODO: IMPLEMENTAR

    // NOTA: el siguiente codigo es solo de prueba. No es valido
    
    pcb_pointer->tabla_paginas = 1;
}

void liberar_estructuras_memoria(void){
    //TODO: IMPLEMENTAR
    return;
}