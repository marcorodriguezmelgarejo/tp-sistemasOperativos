#include "cpu.h"

// TODO: no anda si se hace antes de recibir la conexion del Kernel o si se ejecutaron varias consolas en paralelo
void finalizar_CPU(int signal){ 

    pthread_cancel(hilo_ciclo_instruccion);
    pthread_cancel(hilo_dispatch);
    pthread_cancel(hilo_interrupcion);

    matar_kernel(); // manda a kernel la string FIN_CPU

    pthread_mutex_destroy(&mutex_interrupcion);
    pthread_mutex_destroy(&mutex_PCB);

    sem_destroy(&PCB_en_CPU);
    sem_destroy(&CPU_vacia);

    free(en_ejecucion.lista_instrucciones);

    sockets_cerrar(dispatch_socket);
    sockets_cerrar(interrupt_socket);

    log_destroy(logger);
}

int main(){
    // signal(SIGINT, finalizar_CPU);

    crear_logger();
    cargar_config(logger);
    if(!inicializar_semaforos()){
        log_error(logger, "Error en la creacion de los semaforos");
    }
    conectar_con_kernel();

    signal(SIGINT, finalizar_CPU); // lo pongo aca y no al principio como solucion temporal, porque finalizar_CPU no anda si estamos intentando conectarnos con Kernel

    if(pthread_create(&hilo_dispatch, NULL, (void*) esperar_pcb, NULL) != 0){
        log_error(logger, "Error al crear el hilo de dispatch");
    }
    if(pthread_create(&hilo_ciclo_instruccion, NULL, (void*) ciclo_instruccion, NULL) != 0){
        log_error(logger, "Error al crear el hilo de ciclo de instruccion");
    }
    if(pthread_create(&hilo_interrupcion, NULL, (void*) esperar_interrupcion, NULL) != 0){
        log_error(logger, "Error al crear el hilo de interrupcion");
    }
    
    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_ciclo_instruccion, NULL);
    pthread_join(hilo_interrupcion, NULL);
}
