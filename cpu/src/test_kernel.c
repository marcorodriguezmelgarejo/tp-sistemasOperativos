#include "sockets.h"
#include "protocolos.h"
#include <pthread.h>

t_log* logger;
int dispatch_socket;
int interrupt_socket;
pcb_t en_ejecucion;

#define PUERTO_DISPATCH "8001"
#define CONSOLA_BACKLOG 5
#define PUERTO_INTERRUPT "8005"
#define TIEMPO_EJECUCION 2
#define MOTIVO "EXIT"

void crear_logger(void);
void *conectar_dispatch(void *);
void *conectar_interrupt(void*);
void probar_enviar_pcb(void);
int contar_instrucciones(pcb_t);
void llegar_instruccion_exit(void);
void *escuchar_dispatch(void *);
void *escuchar_interrupt(void *);
void enviar_dispatch(pcb_t);

void crear_logger(void){

	if((logger = log_create("./cfg/cpu.log", "cpu_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");
}

void *conectar_dispatch(void * arg){

    int temp_socket = 0;

    sockets_abrir_servidor(PUERTO_DISPATCH, CONSOLA_BACKLOG, &temp_socket, logger);
    
    sockets_esperar_cliente(temp_socket, &dispatch_socket, logger);
    
    sockets_cerrar(temp_socket); //cierro el servidor porque no espero mas clientes

    return NULL;
}

void *conectar_interrupt(void *arg){

    int temp_socket_1 = 0;

    sockets_abrir_servidor(PUERTO_INTERRUPT, CONSOLA_BACKLOG, &temp_socket_1, logger);
    
    sockets_esperar_cliente(temp_socket_1, &interrupt_socket, logger);
    
    sockets_cerrar(temp_socket_1); //cierro el servidor porque no espero mas clientes

    return NULL;
}

void probar_enviar_pcb(void){
    //ya se probo esto

    pcb_t pcb = {
    1,
    64,
    0,
    malloc(1000),
    -1,
    10000,
    };

    char instruccion[50] = "READ 1 50\nEXIT\n";
    pcb.lista_instrucciones = instruccion;

    while(true){
        sockets_enviar_string(dispatch_socket, "I/0", logger);
        sockets_enviar_pcb(dispatch_socket, pcb, logger);
        pcb.estimacion_rafaga = 15000;
        sleep(5);
    }
    free(pcb.lista_instrucciones);
}

int contar_instrucciones(pcb_t pcb){
    return strspn(pcb.lista_instrucciones, "\n") + 1;
}

void llegar_instruccion_exit(void){
    
    sleep(TIEMPO_EJECUCION);

    en_ejecucion.program_counter = contar_instrucciones(en_ejecucion);

    log_info(logger, "pid = %d llego a instruccion EXIT", en_ejecucion.pid);

    enviar_dispatch(en_ejecucion);

    en_ejecucion.pid = -1;
}

void *escuchar_dispatch(void *arg){

    pcb_t pcb_buffer;

    while(1){

        pcb_buffer.lista_instrucciones = NULL;
        sockets_recibir_pcb(dispatch_socket, &pcb_buffer, logger);

        if (en_ejecucion.pid != -1){
            log_error(logger, "Se recibio pcb pero ya hay uno en ejecucion");
        }

        log_info(logger, "Se recibio pcb (pid = %d)", pcb_buffer.pid);
        en_ejecucion = pcb_buffer;
        log_info(logger, "Se esta ejecutando pid = %d", en_ejecucion.pid);

        llegar_instruccion_exit();

        free(pcb_buffer.lista_instrucciones);
    }

    return NULL;
}

void enviar_dispatch(pcb_t pcb_a_enviar){
    
    sockets_enviar_string(dispatch_socket, MOTIVO, logger);

    sockets_enviar_pcb(dispatch_socket, pcb_a_enviar, logger);
}

void *escuchar_interrupt(void * arg){

    int32_t buffer = 0;

    while(1){
        sockets_recibir_dato(interrupt_socket, &buffer, sizeof buffer, logger);
        log_info(logger, "Se recibio interrupcion del kernel");

        en_ejecucion.program_counter++; //aumento en uno simbolicamente

        enviar_dispatch(en_ejecucion);

        en_ejecucion.pid = -1;
    }

    return NULL;
}

int main(){

    // PARA TESTEAR EL KERNEL
    pthread_t h1, h2, h3, h4;
    
    en_ejecucion.pid = -1;

    crear_logger();

    pthread_create(&h1, NULL, conectar_dispatch, NULL);

    pthread_create(&h2, NULL, conectar_interrupt, NULL);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);

    pthread_create(&h3, NULL, escuchar_dispatch, NULL);
    pthread_create(&h4, NULL, escuchar_interrupt, NULL);

    pthread_join(h3, NULL);
    pthread_join(h4, NULL);

    sockets_cerrar(dispatch_socket);
    sockets_cerrar(interrupt_socket);

    return 0;
}
