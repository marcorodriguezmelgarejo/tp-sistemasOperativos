#include "cpu.h"
#include "cpu.h"

int main(){
    crear_logger();
    cargar_config(logger);
    inicializar_semaforos();
    // conectar_dispatch();
    // conectar_interrupt();

    pthread_t hilo_dispatch;
    pthread_t hilo_ciclo_instruccion;
    pthread_t hilo_interrupcion;

    ciclo_instruccion();

    // pthread_create(&hilo_dispatch, NULL, (void*) esperar_pcb, NULL);
    // if(pthread_create(&hilo_ciclo_instruccion, NULL, (void*) ciclo_instruccion, NULL) == 0){
    //     log_debug(logger, "Hilo creado con exito");
    // }else{
    //     log_error(logger, "Error al crear el hilo");
    // }
    // pthread_create(&hilo_interrupcion, NULL, (void*) esperar_int, NULL);
    
    // pthread_join(hilo_dispatch, NULL);
    // pthread_join(hilo_ciclo_instruccion, NULL);
    // pthread_join(hilo_interrupcion, NULL);
    // TODO: capturar sigint para liberar memoria cuando se interrumpa con CTRL+C (destruir semaforos, logger, free(lista_instrucciones), etc)
}


