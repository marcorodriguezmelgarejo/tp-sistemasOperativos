#include "memoria.h"

void manejar_sigint(int signal){
    liberar_memoria();
}

void liberar_memoria(void){
    
    log_info(logger, "liberando memoria...");

    free(bitarray_aux);
    free(espacio_usuario);

    sem_destroy(&contador_cola_instrucciones_swap);
    pthread_mutex_destroy(&mutex_cola_instrucciones_swap);

    bitarray_destroy(marcos_libres);
    
    log_destroy(logger);

    dictionary_destroy(diccionario_tabla_pointers);
}

void crear_hilos(void){
    pthread_create(&h1, NULL, hilo_escuchar_cpu, NULL);
    pthread_create(&h2, NULL, hilo_escuchar_kernel, NULL);
    pthread_create(&h3, NULL, hilo_swap, NULL);
}

void inicializar_variables_globales(void){

    //primero cargar config antes de llamar a esta funcion

    cola_instrucciones_swap = queue_create();

    pthread_mutex_init(&mutex_cola_instrucciones_swap, NULL);
    
    sem_init(&contador_cola_instrucciones_swap, 0, 0);

    diccionario_tabla_pointers = dictionary_create();

    // Inicializo el bitmap de los marcos libres en espacio de usuario

    if (((TAM_MEMORIA/TAM_PAGINA)%8) != 0 ){
        log_error(logger, "La cantidad de marcos del espacio de usuario tiene que ser divisible por 0");
    }

    int32_t tamanio_marcos_libres = ceil((TAM_MEMORIA/TAM_PAGINA)/8);

    bitarray_aux = malloc(tamanio_marcos_libres);

    marcos_libres = bitarray_create_with_mode(bitarray_aux, tamanio_marcos_libres, MSB_FIRST);
}

int main() {

    signal(SIGINT, manejar_sigint);

	logger = crear_logger();

	cargar_config();

    inicializar_variables_globales();

    espacio_usuario = malloc(TAM_MEMORIA);

    log_debug(logger, "Se aloco la memoria de usuario correspondiente.", TAM_MEMORIA);
    log_debug(logger, "Tamanio total de la memoria: %d Bytes.", TAM_MEMORIA);
    log_debug(logger, "Tamanio de cada pagina: %d Bytes.", TAM_PAGINA);
    log_debug(logger, "Cantidad de marcos en espacio de usuario: %d", bitarray_get_max_bit(marcos_libres));

    if(TAM_MEMORIA%TAM_PAGINA != 0){ //Chequeamos que sea divisible (resto 0)
        log_error(logger, "El tamaño de la pagina no es utilizable para este tamanio de memoria.");
		return ERROR_STATUS;
    }

    iniciar_test();

    conectar_cpu_y_kernel();

    crear_hilos();
	
    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    pthread_join(h3, NULL);

	return SUCCESS_STATUS;
}
