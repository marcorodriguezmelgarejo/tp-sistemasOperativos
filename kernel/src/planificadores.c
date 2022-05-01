#include "kernel.h"

void planificador_largo_plazo_ready(void){
    /*
        Gestiona la transicion NEW->READY
    */
    
    pcb_t **pcb_pointer;

    if (grado_multiprogramacion_actual < GRADO_MULTIPROGRAMACION){

        pcb_pointer = queue_peek(cola_new);
        inicializar_estructuras_memoria();
        list_add(lista_ready, queue_pop(cola_new));

        log_debug(logger, "NEW->READY (pid = %d)", (*pcb_pointer)->pid);

        grado_multiprogramacion_actual++;
    }
}

void planificador_largo_plazo_exit(void){
    /*
        Gestiona la transicion EJEC->EXIT
    */

    liberar_estructuras_memoria();

    finalizar_conexion_consola(en_ejecucion->pid);

    en_ejecucion = NULL;
    grado_multiprogramacion_actual--;
}
