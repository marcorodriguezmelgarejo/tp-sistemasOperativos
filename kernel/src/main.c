#include "kernel.h"

void manejar_sigint(int signal){
    /*
        captura sigint para hacer cleanup
    */

    enviar_fin_cpu();

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

    liberar_memoria_lista_pcb(lista_bloqueado_suspendido);
    list_destroy(lista_bloqueado_suspendido);

    liberar_memoria_cola_pcb(cola_ready_suspendido);
    queue_destroy(cola_ready_suspendido);

    liberar_memoria_cola_pcb(cola_datos_bloqueo);
    queue_destroy(cola_datos_bloqueo);

    log_destroy(logger);

    pthread_mutex_destroy(&mutex_cola_datos_bloqueo);
    pthread_mutex_destroy(&mutex_cola_new);
    pthread_mutex_destroy(&mutex_lista_ready);
    pthread_mutex_destroy(&mutex_lista_bloqueado);
    pthread_mutex_destroy(&mutex_lista_bloqueado_suspendido);
    pthread_mutex_destroy(&mutex_cola_ready_suspendido);
    pthread_mutex_destroy(&mutex_en_ejecucion);
    pthread_mutex_destroy(&mutex_grado_multiprogramacion_actual);
    pthread_mutex_destroy(&mutex_interrupcion_cpu);
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
    
    pthread_mutex_init(&mutex_cola_datos_bloqueo, NULL);
    pthread_mutex_init(&mutex_cola_new, NULL);
    pthread_mutex_init(&mutex_lista_ready, NULL);
    pthread_mutex_init(&mutex_lista_bloqueado, NULL);
    pthread_mutex_init(&mutex_lista_bloqueado_suspendido, NULL);
    pthread_mutex_init(&mutex_cola_ready_suspendido, NULL);
    pthread_mutex_init(&mutex_en_ejecucion, NULL);
    pthread_mutex_init(&mutex_grado_multiprogramacion_actual, NULL);
    pthread_mutex_init(&mutex_interrupcion_cpu, NULL);

    cola_new = queue_create();
    lista_ready = list_create();
    lista_bloqueado = list_create();
    lista_bloqueado_suspendido = list_create();
    cola_ready_suspendido = queue_create();

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

    ya_se_envio_interrupcion_cpu = false;
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

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
        
    return 0;
}
