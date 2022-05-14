#include "kernel.h"

void testear_seleccionar_proceso_menor_estimacion(void){
    pcb_t pid1 = {
    1,
    64,
    0,
    NULL,
    -1,
    54,
    0,
    0,
    10
    };
    pcb_t pid2 = {
    2,
    64,
    0,
    NULL,
    -1,
    1000,
    0,
    0,
    10
    };

    pcb_t pid3 = {
    3,
    64,
    0,
    NULL,
    -1,
    1,
    0,
    0,
    10
    };

    pcb_t pid4 = {
    4,
    64,
    0,
    NULL,
    -1,
    10000,
    0,
    0,
    10
    };

    pcb_t * pcb_menor_estimacion = NULL;

    todos_pcb[todos_pcb_length] = pid1;
    list_add(lista_ready, &(todos_pcb[todos_pcb_length]));
    todos_pcb_length++;

    todos_pcb[todos_pcb_length] = pid2;
    list_add(lista_ready, &(todos_pcb[todos_pcb_length]));
    todos_pcb_length++;

    todos_pcb[todos_pcb_length] = pid3;
    list_add(lista_ready, &(todos_pcb[todos_pcb_length]));
    todos_pcb_length++;

    todos_pcb[todos_pcb_length] = pid4;
    list_add(lista_ready, &(todos_pcb[todos_pcb_length]));
    todos_pcb_length++;

    printf("breakpoint 1\n");

    pcb_menor_estimacion = seleccionar_proceso_menor_estimacion();

    printf("breakpoint 2\n");

    log_info(logger, "PCB con menor estimacion: PID=%d", pcb_menor_estimacion->pid);
    
    if (pcb_menor_estimacion->pid = 3) log_info(logger, "seleccionar_proceso_menor_estimacion() anda bien\n");
    else log_error(logger, "seleccionar_proceso_menor_estimacion() anda MAL\n");
}