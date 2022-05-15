#include "kernel.h"

void manejar_sigint(int signal){
    /*
        captura sigint para hacer cleanup
    */

    pthread_cancel(h1);
    pthread_cancel(h2);
    liberar_memoria();
}

void liberar_memoria(void){

    log_info(logger, "Liberando memoria...");

    liberar_memoria_cola_pcb(cola_new);
    queue_destroy(cola_new);

    liberar_memoria_lista_pcb(lista_ready);
    list_destroy(lista_ready);

    liberar_memoria_lista_pcb(lista_bloqueado);
    list_destroy(lista_bloqueado);

    liberar_memoria_lista_pcb(lista_bloqueado_sus);
    list_destroy(lista_bloqueado_sus);

    liberar_memoria_cola_pcb(cola_ready_sus);
    queue_destroy(cola_ready_sus);

    log_destroy(logger);
}

void liberar_memoria_lista_pcb(t_list* lista){

    int i = 0;
    pcb_t* pcb_pointer;

    for (i = 0; i < list_size(lista); i++){
        pcb_pointer = list_remove(lista, i);
        free(pcb_pointer->lista_instrucciones);
        free(pcb_pointer);
    }

}

void liberar_memoria_cola_pcb(t_queue* cola){

    int i = 0;
    pcb_t * pcb_pointer;

    for (i = 0; i < queue_size(cola); i++){
        pcb_pointer = queue_pop(cola);
        free(pcb_pointer->lista_instrucciones);
        free(pcb_pointer);
    }
}

void liberar_memoria_pcb(pcb_t * pcb_pointer){
    free(pcb_pointer->lista_instrucciones);
    free(pcb_pointer);
}

void inicializar_estructuras(void){
    
    cola_new = queue_create();
    lista_ready = list_create();
    lista_bloqueado = list_create();
    lista_bloqueado_sus = list_create();
    cola_ready_sus = queue_create();

    crear_logger();
}

void inicializar_variables_globales(void){

    contador_pid = 0;
    
    grado_multiprogramacion_actual = 0;

    consolas_socket = 0;
    dispatch_socket = 0;
    interrupt_socket = 0;

    en_ejecucion = NULL;
}

int main(void)
{

    inicializar_variables_globales();

    signal(SIGINT, manejar_sigint);

    inicializar_estructuras();

    cargar_config();

    //probar_conexion_consola();

    conectar_puerto_dispatch();
    
    conectar_puerto_interrupt();

    //TODO: conectar con MEMORIA

    pthread_create(&h1, NULL, gestionar_dispatch, NULL);
    pthread_create(&h2, NULL, gestionar_nuevas_consolas, NULL);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
        
    return 0;
}
