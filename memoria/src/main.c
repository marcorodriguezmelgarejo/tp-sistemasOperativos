#include "funciones.h"

int main() {

	logger = crear_logger();

	cargar_config();

    // Hay que conectar escucha|envia para el Kernel
	/*if (!sockets_conectar_como_cliente(ip, port, &kernel_socket, logger)){
		log_error(logger, "Error al intentar conectar con kernel. Finalizando...");
		salir_error(logger, NULL);
	}*/

    void* memory_start = malloc(TAM_MEMORIA);
    log_info(logger, "Se ha alocado la memoria con posicion inicial en %p.", memory_start);
    log_info(logger, "Tamanio total de la memoria: %d Bytes.", TAM_MEMORIA);
    log_info(logger, "Tamanio de cada pagina: %d Bytes.", TAM_PAGINA);

    if(TAM_MEMORIA%TAM_PAGINA == 0){ //Chequeamos que sea divisible (resto 0)
        uint32_t cant_pages = TAM_MEMORIA/TAM_PAGINA;
        log_info(logger, "Cantidad de paginas: %d Bytes.", cant_pages);

    } else {
        log_error(logger, "El tamaño de la pagina no es utilizable para este tamanio de memoria.");
		salir_error(logger, NULL);
    }

    // Arrancar la division de paginacion
    

    log_info(logger, "Se ha terminado de ejecutar el programa. Finalizando.");
	//finalizar_programa(kernel_socket, logger);

	return SUCCESS_STATUS;
}
