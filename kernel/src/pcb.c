#include "kernel.h"

void generar_pcb(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
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
        return;
    }

    *pcb_pointer = pcb_nuevo;

    queue_push(cola_new, pcb_pointer);

    contador_pid++;

    log_debug(logger, "Nuevo proceso en NEW (pid = %d)", contador_pid - 1);
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

void actualizar_timestamp(pcb_t * pcb_pointer){

    struct timeval tv;

    gettimeofday(&tv, NULL);
    
    pcb_pointer->timestamp = (uint64_t) (tv.tv_sec * 1000) + (uint64_t) (tv.tv_usec/1000);
}

int32_t get_tiempo_transcurrido(uint64_t timestamp_anterior){
    /*
        devuelve el tiempo transcurrido entre el tiempo actual y el tiempo pasado como argumento
    */
    struct timeval tv;
    uint64_t timestamp_actual;

    gettimeofday(&tv, NULL);

    timestamp_actual = (uint64_t) (tv.tv_sec * 1000) + (uint64_t) (tv.tv_usec/1000);

    return (int32_t) (timestamp_actual - timestamp_anterior);
}