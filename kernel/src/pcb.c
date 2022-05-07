#include "kernel.h"

void generar_pcb(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la generacion de un nuevo pcb
    */

    char pid_string[12];
    pcb_t * pcb_pointer = alocar_memoria_todos_pcb();

    pcb_t pcb_nuevo = {
    contador_pid,
    tamanio_proceso,
    0,
    lista_instrucciones,
    -1,
    ESTIMACION_INICIAL,
    };

    *pcb_pointer = pcb_nuevo;

    queue_push(cola_new, &pcb_pointer);

    sprintf(pid_string, "%d", contador_pid); //para pasar de int a string (para usarse como key del diccionario 'pid_to_socket')
    dictionary_put(pid_to_socket, pid_string, &socket);

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

pcb_t * alocar_memoria_todos_pcb(void){
    /*
        Devuelve un puntero a un area de memoria donde guardar un pcb
    */

    //si nunca se aloco memoria
    if (todos_pcb_length == 0){
        if((todos_pcb = malloc(sizeof *todos_pcb)) == NULL){
            log_error(logger, "Fallo al hacer malloc para 'todos_pcb'");
            return NULL;
        }
    }
    else{
        if((todos_pcb = realloc(todos_pcb, (sizeof *todos_pcb) * (todos_pcb_length + 1))) == NULL){
            log_error(logger, "Fallo al hacer realloc para 'todos_pcb'");
            return NULL;
        }
    }

    todos_pcb_length++;

    return todos_pcb + todos_pcb_length - 1;
}

pcb_t *obtener_pcb_pointer(pcb_t pcb){

    int i = 0;

    for (i = 0; i < todos_pcb_length; i++){
        if ((todos_pcb[i]).pid == pcb.pid){
            return &(todos_pcb[i]);
        }
    }
    
    return NULL;
}

void actualizar_pcb(pcb_t pcb_actualizado){
    //busca el pcb en todos_pcb por pid y lo actualiza
    //actualiza hasta ahora solo el program_counter

    pcb_t* pointer = obtener_pcb_pointer(pcb_actualizado);

    pointer->program_counter = pcb_actualizado.program_counter;
}