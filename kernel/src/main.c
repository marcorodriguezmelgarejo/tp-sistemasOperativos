#include "kernel.h"

void manejar_sigint(int signal){
    /*
        captura sigint para hacer cleanup
    */

    pthread_cancel(h1);
    pthread_cancel(h2);
    pthread_cancel(h3);
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

    liberar_memoria_lista_pcb(lista_bloqueado_suspendido);
    list_destroy(lista_bloqueado_suspendido);

    liberar_memoria_cola_pcb(cola_ready_suspendido);
    queue_destroy(cola_ready_suspendido);

    liberar_threads_cola(cola_threads);
    queue_destroy(cola_threads);

    liberar_memoria_cola_pcb(cola_datos_bloqueo);
    queue_destroy(cola_datos_bloqueo);

    log_destroy(logger);

    pthread_mutex_destroy(&mutex_cola_threads);
    pthread_mutex_destroy(&mutex_cola_datos_bloqueo);
}

void liberar_threads_cola(t_queue* cola){

    pthread_t * thread_pointer = NULL;

    if(!queue_is_empty(cola)){
        thread_pointer = queue_pop(cola);
        pthread_join(*thread_pointer, NULL);
    }
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
    
    sem_init(&semaforo_cola_threads, 0, 0);
    pthread_mutex_init(&mutex_cola_threads, NULL);
    pthread_mutex_init(&mutex_cola_datos_bloqueo, NULL);

    cola_new = queue_create();
    lista_ready = list_create();
    lista_bloqueado = list_create();
    lista_bloqueado_suspendido = list_create();
    cola_ready_suspendido = queue_create();

    cola_threads = queue_create();
    cola_datos_bloqueo = queue_create();

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

    signal(SIGUSR1, ingresar_proceso_a_ready);

    inicializar_estructuras();

    cargar_config();

    //probar_conexion_consola();

    conectar_puerto_dispatch();
    
    conectar_puerto_interrupt();

    //TODO: conectar con MEMORIA

    pthread_create(&h1, NULL, gestionar_dispatch, NULL);
    pthread_create(&h2, NULL, gestionar_nuevas_consolas, NULL);
    pthread_create(&h3, NULL, hacer_join_hilos_mediano_plazo, NULL);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    pthread_join(h3, NULL);
        
    return 0;
}
