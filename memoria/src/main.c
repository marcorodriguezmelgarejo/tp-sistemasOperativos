#include "memoria.h"

void salir_error(t_log* logger, int* kernel_socket_pointer){
	if (kernel_socket_pointer != NULL) sockets_cerrar(*kernel_socket_pointer);
	if (logger != NULL) log_destroy(logger);
	exit(ERROR_STATUS);
}

void crear_hilos(void){
    pthread_create(&h1, NULL, hilo_escuchar_cpu, NULL);
    pthread_create(&h2, NULL, hilo_escuchar_kernel, NULL);
    pthread_create(&h3, NULL, hilo_swap, NULL);
}

int main() {

	logger = crear_logger();

	cargar_config();

    espacio_usuario = malloc(TAM_MEMORIA);

    log_info(logger, "Se ha alocado la memoria con posicion inicial en %p.", espacio_usuario);
    log_info(logger, "Tamanio total de la memoria: %d Bytes.", TAM_MEMORIA);
    log_info(logger, "Tamanio de cada pagina: %d Bytes.", TAM_PAGINA);

    if(TAM_MEMORIA%TAM_PAGINA == 0){ //Chequeamos que sea divisible (resto 0)
        cantidad_total_marcos = TAM_MEMORIA/TAM_PAGINA;
        log_info(logger, "Cantidad total de marcos: %d", cantidad_total_marcos);

    } else {
        log_error(logger, "El tamaño de la pagina no es utilizable para este tamanio de memoria.");
		salir_error(logger, NULL);
    }

    conectar_cpu_y_kernel();

    crear_hilos();
	
    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    pthread_join(h3, NULL);

	return SUCCESS_STATUS;
}
