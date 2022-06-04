#include "kernel.h"

pcb_t *generar_pcb(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la generacion de un nuevo pcb
    */

    pcb_t * pcb_pointer = NULL;

    pcb_t pcb_nuevo = {
    contador_pid,
    tamanio_proceso,
    0,
    lista_instrucciones,
    -1,
    ESTIMACION_INICIAL,
    0,
    0,
    socket
    };

    if ((pcb_pointer = alocar_memoria_pcb()) == NULL){
        log_error(logger, "Error al generar_pcb()");
        return NULL;
    }

    *pcb_pointer = pcb_nuevo;

    contador_pid++;

    return pcb_pointer;
}

void agregar_instruccion_a_lista(char ** lista, char* instruccion){

    instruccion[strcspn(instruccion, "\n")] = '\0'; //saco si hay el \n

    //si todavia no se agrego ninguna instruccion
    if (*lista == NULL){
        if ((*lista = malloc(strlen(instruccion) + 1)) == NULL){
            log_error(logger, "Error al hacer malloc en agregar_instruccion_a_lista()");
        }
        strcpy(*lista, instruccion);
    }
    else{
        if ((*lista = realloc(*lista, strlen(*lista) + strlen(instruccion) + 2)) == NULL){
            log_error(logger, "Error al hacer realloc en agregar_instruccion_a_lista()");
        }
        
        //concateno la instruccion nueva con todas las instrucciones anteriores
        strcat(*lista, "\n");
        strcat(*lista, instruccion);
    }
}

pcb_t * alocar_memoria_pcb(void){
    /*
        Devuelve un puntero a un area de memoria donde guardar un pcb
    */

    pcb_t * pcb_pointer = NULL;

    if((pcb_pointer = malloc(sizeof *pcb_pointer)) == NULL){
        log_error(logger, "Fallo al hacer malloc en 'alocar_memoria_todos_pcb()'");
        return NULL;
    }

    return pcb_pointer;
}

void actualizar_program_counter_en_ejecucion(int32_t program_counter){

    if (en_ejecucion == NULL){
        log_error(logger, "Error en actualizar_program_counter_en_ejecucion(): 'en_ejecucion' es NULL");
    }

    en_ejecucion->program_counter = program_counter;
}

void sumar_duracion_rafaga(pcb_t * pcb_pointer, int32_t tiempo_ejecucion){
    pcb_pointer->duracion_real_ultima_rafaga += tiempo_ejecucion;
}

int32_t calcular_estimacion_rafaga(pcb_t *pcb_pointer){
    
    int32_t estimacion_anterior = pcb_pointer->estimacion_rafaga;
    int32_t duracion_real = pcb_pointer->duracion_real_ultima_rafaga;

    return (int32_t) (ALFA * duracion_real + (1-ALFA) * estimacion_anterior);
}