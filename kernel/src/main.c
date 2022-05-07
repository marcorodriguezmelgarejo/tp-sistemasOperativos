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

    int i = 0;

    log_info(logger, "Liberando memoria...");

    if (todos_pcb != NULL){

        //libero todas las listas de instrucciones de los pcb
        for (i = 0; i < todos_pcb_length; i++){
            free((todos_pcb + i)->lista_instrucciones);
        }

        free(todos_pcb);
    }

    dictionary_destroy(pid_to_socket);
    
    queue_destroy(cola_new);
    list_destroy(lista_ready);
    list_destroy(lista_bloqueado);
    list_destroy(lista_bloqueado_sus);
    queue_destroy(cola_ready_sus);

    log_destroy(logger);
}

void inicializar_estructuras(void){

    pid_to_socket = dictionary_create();
    
    cola_new = queue_create();
    lista_ready = list_create();
    lista_bloqueado = list_create();
    lista_bloqueado_sus = list_create();
    cola_ready_sus = queue_create();

    crear_logger();
}

int main(void)
{
    //Inicializo variables globales

    contador_pid = 0;
    todos_pcb = NULL;
    todos_pcb_length = 0;
    
    grado_multiprogramacion_actual = 0;

    consolas_socket = 0;
    dispatch_socket = 0;
    interrupt_socket = 0;

    signal(SIGINT, manejar_sigint);

    inicializar_estructuras();

    cargar_config();

    conectar_puerto_dispatch();
    
    conectar_puerto_interrupt();

    //TODO: conectar con MEMORIA

    pthread_create(&h1, NULL, gestionar_dispatch, NULL);
    pthread_create(&h2, NULL, gestionar_nuevas_consolas, NULL);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    //probar_conexion_consola();

    return 0;
}
