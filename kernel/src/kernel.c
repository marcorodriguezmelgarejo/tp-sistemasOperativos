#include "kernel.h"

void crear_logger(void){

	if((logger = log_create(LOG_FILENAME,"kernel_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(1);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");
}

void manejar_sigint(int signal){
    /*
        captura sigint para liberar memoria
    */

    liberar_memoria();
    printf("Finalizando...\n");
}

void * escuchar_nuevas_consolas(void * arg){
    /*
        Gestiona la conexion con nuevas consolas
    */

    char buffer[MAX_INSTRUCCION_SIZE];
    int socket_consola = 0;
    char instruccion_o_tamanio[12];
    int32_t tamanio_proceso = 0;
    char *lista_instrucciones = NULL;

    while(1){
        sockets_esperar_cliente(socket_server, &socket_consola, logger);

        do{
            sockets_recibir_string(socket_consola, instruccion_o_tamanio, logger);

            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(socket_consola, buffer, logger);
                agregar_instruccion_a_lista(&lista_instrucciones, buffer);
            }
            else{
                sockets_recibir_dato(socket_consola, &tamanio_proceso, sizeof (int32_t), logger);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

        generar_pcb(lista_instrucciones, tamanio_proceso, socket_consola);
        lista_instrucciones = NULL;
    }

    return NULL;
}

void generar_pcb(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la generacion de un nuevo pcb
    */

    char pid_string[12];
    pcb_t * pcb_pointer = alocar_memoria_todos_pcb();

    pcb_t pcb_nuevo = {
    contador_pid,
    tamanio_proceso,
    0,
    lista_instrucciones,
    -1,
    ESTIMACION_INICIAL,
    };

    *pcb_pointer = pcb_nuevo;

    queue_push(cola_new, &pcb_pointer);

    sprintf(pid_string, "%d", contador_pid); //para pasar de int a string (para usarse como key del diccionario 'pid_to_socket')
    dictionary_put(pid_to_socket, pid_string, &socket);

    contador_pid++;

    log_debug(logger, "Se genero exitosamente el pcb con pid %d", contador_pid - 1);
}

void agregar_instruccion_a_lista(char ** lista, char* instruccion){

    instruccion[strcspn(instruccion, "\n")] = '\0'; //saco si hay el \n

    //si todavia no se agrego ninguna instruccion
    if (*lista == NULL){
        if ((*lista = malloc(strlen(instruccion) + 1)) == NULL){
            log_error(logger, "Error al hacer malloc en agregar_instruccion_a_lista()");
        }
        strcpy(*lista, instruccion);
    }
    else{
        if ((*lista = realloc(*lista, strlen(*lista) + strlen(instruccion) + 2)) == NULL){
            log_error(logger, "Error al hacer realloc en agregar_instruccion_a_lista()");
        }
        
        //concateno la instruccion nueva con todas las instrucciones anteriores
        strcat(*lista, "\n");
        strcat(*lista, instruccion);
    }
}

void liberar_memoria(void){

    int i = 0;

    if (todos_pcb != NULL){

        //libero todas las listas de instrucciones de los pcb
        for (i = 0; i < todos_pcb_length; i++){
            free((todos_pcb + i)->lista_instrucciones);
        }

        free(todos_pcb);
    }

    sockets_cerrar(socket_server);

    dictionary_destroy(pid_to_socket);
    
    queue_destroy(cola_new);
    queue_destroy(cola_ready);
    list_destroy(lista_bloqueado);
    list_destroy(lista_bloqueado_sus);
    queue_destroy(cola_ready_sus);
    queue_destroy(cola_exit);

    log_destroy(logger);
}

pcb_t * alocar_memoria_todos_pcb(void){
    /*
        Devuelve un puntero a un area de memoria donde guardar un pcb
    */

    //si nunca se aloco memoria
    if (todos_pcb_length == 0){
        if((todos_pcb = malloc(sizeof *todos_pcb)) == NULL){
            log_error(logger, "Fallo al hacer malloc para 'todos_pcb'");
            return NULL;
        }
    }
    else{
        if((todos_pcb = realloc(todos_pcb, (sizeof *todos_pcb) * (todos_pcb_length + 1))) == NULL){
            log_error(logger, "Fallo al hacer realloc para 'todos_pcb'");
            return NULL;
        }
    }

    todos_pcb_length++;

    return todos_pcb + todos_pcb_length - 1;
}

void finalizar_conexion_consola(int32_t pid){
    /*
        Manda mensaje de finalizacion a la consola y cierra el socket correspondiente
    */

    int * socket_pointer = NULL;

    char pid_string[12];

    int32_t dummy = DUMMY_VALUE;

    sprintf(pid_string, "%d", pid); //convierte el pid a string para poder ser key del diccionario 'pid_to_socket'

    if ((socket_pointer = dictionary_get(pid_to_socket, pid_string)) == NULL){
        log_error(logger, "No se encontró el socket correspondiente al pid %u", pid);
        return;
    }

    if (sockets_enviar_dato(*socket_pointer, &dummy, sizeof(int32_t), logger) == false){
	    log_error(logger, "Error al comunicarse con consola pid %u", pid);
    }

    sockets_cerrar(*socket_pointer);

    log_info(logger, "Se cerró la conexion con consola pid %u", pid);
}

void inicializar_estructuras(void){

    pid_to_socket = dictionary_create();
    
    cola_new = queue_create();
    cola_ready = queue_create();
    lista_bloqueado = list_create();
    lista_bloqueado_sus = list_create();
    cola_ready_sus = queue_create();
    cola_exit = queue_create();

    crear_logger();
}

void cargar_config(void){
    //TODO:implementar
    return;
}

void probar_conexion_consola(void){
    /*
        Es para testear
    */
    pthread_t h1;

    inicializar_estructuras();

    sockets_abrir_servidor("8000", CONSOLA_BACKLOG, &socket_server, logger);

    cargar_config();

    pthread_create(&h1, NULL, escuchar_nuevas_consolas, NULL);

    
    sleep(7);

    int i = 0;
    printf("todos_pcb_length: %d\n", todos_pcb_length);
    for (i = 0; i < todos_pcb_length; i++){
        printf("\nLista instrucciones: %s\n", (todos_pcb[i]).lista_instrucciones);
    }

    sleep(60);

    liberar_memoria();
}

int main(void)
{
    signal(SIGINT, manejar_sigint);
    probar_conexion_consola();

    return 0;
}
