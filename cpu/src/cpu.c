#include "cpu.h"
#include "cpu.h"

int main(){
    pthread_t hilo_dispatch, hilo_ciclo_instruccion, hilo_interrupcion;

    crear_logger();
    cargar_config(logger);
    if(inicializar_semaforos()){
        log_debug(logger, "Semaforos creados correctamente");
    }else{
        log_error(logger, "Error en la creacion de los semaforos");
    }
    conectar_con_kernel();

    if(pthread_create(&hilo_dispatch, NULL, (void*) esperar_pcb, NULL) == 0){
        log_debug(logger, "Hilo dispatch creado con exito");
    }else{
        log_error(logger, "Error al crear el hilo de dispatch");
    }
    if(pthread_create(&hilo_ciclo_instruccion, NULL, (void*) ciclo_instruccion, NULL) == 0){
        log_debug(logger, "Hilo ciclo de instruccion creado con exito");
    }else{
        log_error(logger, "Error al crear el hilo de ciclo de instruccion");
    }
    if(pthread_create(&hilo_interrupcion, NULL, (void*) esperar_interrupcion, NULL) == 0){
        log_debug(logger, "Hilo interrupcion creado con exito");
    }else{
        log_error(logger, "Error al crear el hilo de interrupcion");
    }
    
    
    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_ciclo_instruccion, NULL);
    pthread_join(hilo_interrupcion, NULL);
    // TODO: capturar sigint para liberar memoria cuando se interrumpa con CTRL+C (destruir semaforos, logger, free(lista_instrucciones), etc)
}


