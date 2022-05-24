#include "sockets.h"
#include "protocolos.h"
#include <pthread.h>

t_log* logger;
int dispatch_socket;
int interrupt_socket;

#define PUERTO_DISPATCH "8001"
#define CONSOLA_BACKLOG 5
#define PUERTO_INTERRUPT "8005"

// void crear_logger(void){

// 	if((logger = log_create("./cfg/cpu.log", "cpu_log",1,LOG_LEVEL_TRACE)) == NULL){
// 		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
// 		exit(1);
// 	}

// 	log_debug(logger, "Se ha creado el archivo de log con exito.");
// }

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

void probar_enviar_pcb(){
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

// int main(){

//     // PARA TESTEAR EL KERNEL
//     pthread_t h1, h2;

//     crear_logger();

//     pthread_create(&h1, NULL, conectar_dispatch, NULL);

//     pthread_create(&h2, NULL, conectar_interrupt, NULL);

//     //espero a que finalicen los threads
//     pthread_join(h1, NULL);
//     pthread_join(h2, NULL);

//     sockets_cerrar(dispatch_socket);
//     sockets_cerrar(interrupt_socket);

//     return 0;
// }
