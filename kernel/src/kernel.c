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

void *gestionar_dispatch(void *arg){
    /*
        Abre un servidor y escucha a la espera de mensajes del CPU
    */

    int temp_socket = 0;
    char io_o_exit[5];
    pcb_t pcb_buffer;

    sockets_abrir_servidor(PUERTO_CPU_DISPATCH, CONSOLA_BACKLOG, &temp_socket, logger);

    sockets_esperar_cliente(dispatch_socket, &dispatch_socket, logger);

    sockets_cerrar(temp_socket); //cierro el servidor porque no espero mas clientes

    while(1){

        //recibe la razon del mensaje (si el proceso se fue a i/o o llego a instruccion "EXIT")
        sockets_recibir_string(dispatch_socket, io_o_exit, logger);

        //sockets_recibir_pcb(dispatch_socket, &pcb_buffer, logger);
    }

}

void *gestionar_nuevas_consolas(void * arg){
    /*
        Abre un servidor y escucha nuevas conexiones de consolas
    */

    char buffer[MAX_INSTRUCCION_SIZE];
    int socket_consola_actual = 0;
    char instruccion_o_tamanio[12];
    int32_t tamanio_proceso = 0;
    char *lista_instrucciones = NULL;

    sockets_abrir_servidor(PUERTO_ESCUCHA, CONSOLA_BACKLOG, &consolas_socket, logger);

    while(1){
        sockets_esperar_cliente(consolas_socket, &socket_consola_actual, logger);

        do{
            sockets_recibir_string(socket_consola_actual, instruccion_o_tamanio, logger);

            if (strcmp(instruccion_o_tamanio, "INSTRUCCION") == 0){
                sockets_recibir_string(socket_consola_actual, buffer, logger);
                agregar_instruccion_a_lista(&lista_instrucciones, buffer);
            }
            else{
                sockets_recibir_dato(socket_consola_actual, &tamanio_proceso, sizeof (int32_t), logger);
            }
        }
        while(strcmp(instruccion_o_tamanio, "TAMANIO") != 0);

        generar_pcb(lista_instrucciones, tamanio_proceso, socket_consola_actual);
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

    log_info(logger, "Liberando memoria...");

    if (todos_pcb != NULL){

        //libero todas las listas de instrucciones de los pcb
        for (i = 0; i < todos_pcb_length; i++){
            free((todos_pcb + i)->lista_instrucciones);
        }

        free(todos_pcb);
    }

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
    
    t_config* config;

	if((config = config_create(CONFIG_FILENAME)) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		exit(ERROR_STATUS);
	}

	leer_str_config(config, "IP_MEMORIA", IP_MEMORIA, logger);
	leer_str_config(config, "PUERTO_MEMORIA", PUERTO_MEMORIA, logger);
    leer_str_config(config, "IP_CPU", IP_CPU, logger);
	leer_str_config(config, "PUERTO_CPU_DISPATCH", PUERTO_CPU_DISPATCH, logger);
    leer_str_config(config, "PUERTO_ESCUCHA", PUERTO_ESCUCHA, logger);
	leer_str_config(config, "ALGORITMO_PLANIFICACION", ALGORITMO_PLANIFICACION, logger);
    leer_int_config(config, "ESTIMACION_INICIAL", &ESTIMACION_INICIAL, logger);
	leer_float_config(config, "ALFA", &ALFA, logger);    
    leer_int_config(config, "GRADO_MULTIPROGRAMACION", &GRADO_MULTIPROGRAMACION, logger);
	leer_int_config(config, "TIEMPO_MAXIMO_BLOQUEADO", &TIEMPO_MAXIMO_BLOQUEADO, logger);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
    return;
}

void leer_str_config(t_config* config, char* value, char* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        strcpy(return_value, config_get_string_value(config, value));
	    log_debug(logger, "%s: %s", value, return_value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}
}

void leer_int_config(t_config* config, char* value, int32_t* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        
        *return_value = config_get_int_value(config, value);
		log_debug(logger, "%s: %d", value, *return_value);

	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}   
}

void leer_float_config(t_config* config, char* value, float* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        
        *return_value = config_get_double_value(config, value);
		log_debug(logger, "%s: %f", value, *return_value);

	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}   
}

void probar_conexion_consola(void){
    /*
        Es para testear
    */
    
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
    
    pthread_t h1, h2;

    cargar_config();

    inicializar_estructuras();

    pthread_create(&h1, NULL, gestionar_dispatch, NULL);
    
    //TODO: conectar puerto interrup de CPU como cliente
    //TODO: conectar con MEMORIA

    pthread_create(&h2, NULL, gestionar_nuevas_consolas, NULL);

    probar_conexion_consola();

    return 0;
}
