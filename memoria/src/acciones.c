#include "memoria.h"

/*
    TODO: TESTEAR TODAS LAS FUNCIONES

    funciones testeadas hasta ahora: ninguna
*/

tabla_primer_nivel* inicializar_proceso(int32_t pid, int32_t tamanio_proceso){

    /*
        Inicializa el archivo swap y la tabla de paginas para el proceso
    */

    if (tamanio_proceso >= TAM_PAGINA * ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA){
        log_error(logger, "El tamanio de proceso excede la cantidad de paginas asignables");
    }

    tabla_primer_nivel* tabla_primer_nivel_pointer;
    instruccion_swap instruccion;
    sem_t semaforo;

    int32_t cantidad_paginas = ceil(tamanio_proceso / TAM_PAGINA); //de 1 a ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_primer_nivel = ceil(cantidad_paginas / ENTRADAS_POR_TABLA); //de 1 a ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_segundo_nivel_ultima_entrada = cantidad_paginas % ENTRADAS_POR_TABLA; //de 1 a ENTRADAS_POR_TABLA

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = CREAR_ARCHIVO_SWAP;
    instruccion.pid = pid;
    instruccion.tamanio_proceso = tamanio_proceso;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine la instruccion swap

    sem_destroy(&semaforo);
    
    tabla_primer_nivel_pointer = crear_tabla_paginas_proceso(pid, cantidad_paginas, cantidad_entradas_primer_nivel, cantidad_entradas_segundo_nivel_ultima_entrada);

    return tabla_primer_nivel_pointer;
}

tabla_primer_nivel* crear_tabla_paginas_proceso(int32_t pid, int32_t cantidad_paginas, int32_t cantidad_entradas_primer_nivel, int32_t cantidad_entradas_segundo_nivel_ultima_entrada){
    /*
        Reserva memoria para 1 tabla de paginas de primer nivel y para las tablas de segundo nivel necesarias.
    */

    int i = 0, j = 0, entradas_a_crear = 0;
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    if ((tabla_primer_nivel_pointer = malloc(sizeof *tabla_primer_nivel_pointer)) == NULL){
        log_error(logger, "error al hacer malloc");
        return NULL;
    }

    (*tabla_primer_nivel_pointer).pid = pid;
    (*tabla_primer_nivel_pointer).cantidad_entradas = cantidad_entradas_primer_nivel;
    (*tabla_primer_nivel_pointer).tamanio_conjunto_residente = 0;
    (*tabla_primer_nivel_pointer).puntero_clock = 0;
    (*tabla_primer_nivel_pointer).cantidad_paginas = cantidad_paginas;
    (*tabla_primer_nivel_pointer).lista_de_tabla_segundo_nivel = list_create();

    //reservo memoria e inicializo las tablas de segundo nivel
    for (i = 0; i > cantidad_entradas_primer_nivel; i++){

        if ((tabla_segundo_nivel_pointer = malloc(sizeof *tabla_segundo_nivel_pointer)) == NULL){
            log_error(logger, "error al hacer malloc");
            return NULL;
        }

        //si no es la ultima tabla de segundo nivel  
        if (i != cantidad_entradas_primer_nivel - 1){
            entradas_a_crear = ENTRADAS_POR_TABLA;
        }
        else{
            entradas_a_crear = cantidad_entradas_segundo_nivel_ultima_entrada;
        }

        (*tabla_segundo_nivel_pointer).cantidad_entradas = entradas_a_crear;
        (*tabla_segundo_nivel_pointer).lista_de_entradas = list_create();

        //reservo memoria e inicializo las entradas de segundo nivel
        for (j = 0; j < entradas_a_crear; j++){

            if ((entrada_segundo_nivel_pointer = malloc(sizeof *entrada_segundo_nivel_pointer)) == NULL){
                log_error(logger, "error al hacer malloc");
                return NULL;
            }

            (*entrada_segundo_nivel_pointer).numero_marco = -1;
            (*entrada_segundo_nivel_pointer).presencia = false;
            (*entrada_segundo_nivel_pointer).usado = false;
            (*entrada_segundo_nivel_pointer).modificado = false;

            list_add((*tabla_segundo_nivel_pointer).lista_de_entradas, entrada_segundo_nivel_pointer);
        }

    }

    //agrego el puntero de la tabla de primer nivel a la lista global

    list_add(lista_tabla_primer_nivel, tabla_primer_nivel_pointer);

    return tabla_primer_nivel_pointer;
}

void liberar_memoria_tabla_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Hace todos los frees correspondientes para liberar memoria alocada para una tabla de primer nivel
    */

    int i = 0, index = 0;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;

    for (i = 0; i < tabla_pointer->cantidad_entradas; i++){
        tabla_segundo_nivel_pointer = list_remove(tabla_pointer->lista_de_tabla_segundo_nivel, 0);
        list_destroy_and_destroy_elements(tabla_segundo_nivel_pointer->lista_de_entradas, free);
    }

    list_destroy(tabla_pointer->lista_de_tabla_segundo_nivel);

    if ((index = get_indice_tabla_pointer(lista_tabla_primer_nivel, tabla_pointer)) == -1){
        log_error(logger, "la tabla a liberar no se encuentra en la lista");
        return;
    }

    list_remove(lista_tabla_primer_nivel, index);
}


void suspender_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Envia instruccion a swap para mandar todo el conjunto residente del proceso a disco
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PROCESO_A_DISCO;
    instruccion.tabla_primer_nivel_pointer = tabla_pointer;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);

    return;
}

void finalizar_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Borra el archivo de swap del proceso y libera la tabla de primer nivel del proceso
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    // Borramos el archivo de swap del proceso
    instruccion.numero_instruccion = BORRAR_ARCHIVO_SWAP;
    instruccion.pid = tabla_pointer->pid;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);

    // Liberamos las paginas de memoria principal
    liberar_memoria_tabla_proceso(tabla_pointer);

    return;
}

int32_t acceder_tabla_primer_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){

    // SE DEVUELVE EL INDICE DE LA LISTA tabla_pointer->lista_de_entradas EN DONDE SE ENCUENTRE LA PAGINA
    
    int32_t indice_tabla_segundo_nivel;

    if (numero_pagina < 0){
        log_error(logger, "El numero de pagina no puede ser menor a cero");
        return -1;
    }

    if (excede_la_tabla(tabla_pointer, numero_pagina)){
        log_error(logger, "El numero de pagina excede la cantidad de paginas asignadas al proceso");
        return -1;
    }

    indice_tabla_segundo_nivel = floor(numero_pagina / ENTRADAS_POR_TABLA);

    log_debug(logger, "acceder_tabla_primer_nivel PID = %d, indice devuelto = %d", tabla_pointer->pid, indice_tabla_segundo_nivel);
    
    return indice_tabla_segundo_nivel;
}

int32_t acceder_tabla_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina_solicitada){

    /*
        Retorna el numero de marco en donde se encuentra la pagina solicitada.
        Si la misma no se encuentra en memoria se trae, reemplazando una pagina si es necesario.
    */

    
    bool se_reemplazo_pagina = false;

    int32_t numero_pagina_a_reemplazar;

    entrada_segundo_nivel* entrada_segundo_nivel_a_reemplazar_pointer;

    int32_t numero_marco;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, numero_pagina_solicitada);

    //si la pagina no esta en memoria la traigo desde disco
    if (entrada_segundo_nivel_pointer->presencia == false){

        //si hace falta reemplazar una pagina
        if (tabla_pointer->tamanio_conjunto_residente == MARCOS_POR_PROCESO){
            numero_pagina_a_reemplazar = elegir_pagina_para_reemplazar(tabla_pointer);

            entrada_segundo_nivel_a_reemplazar_pointer = get_entrada_segundo_nivel(tabla_pointer, numero_pagina_a_reemplazar);

            numero_marco = entrada_segundo_nivel_a_reemplazar_pointer->numero_marco;

            //si fue modificada la traslado a disco
            if (entrada_segundo_nivel_a_reemplazar_pointer->modificado == true){
                acciones_trasladar_pagina_a_disco(tabla_pointer->pid, numero_pagina_a_reemplazar, numero_marco);
            }

            entrada_segundo_nivel_a_reemplazar_pointer->presencia = false;

            se_reemplazo_pagina = true;
        }
        else{
            numero_marco = elegir_marco_libre(tabla_pointer);

            if (numero_marco == -1){
                log_error(logger, "No hay mas memoria disponible");
                return -1;
            }
        }

        acciones_trasladar_pagina_a_memoria(tabla_pointer->pid, numero_pagina_solicitada, numero_marco);

        entrada_segundo_nivel_pointer->presencia = true;
        entrada_segundo_nivel_pointer->numero_marco = numero_marco;
        entrada_segundo_nivel_pointer->usado = true;
        entrada_segundo_nivel_pointer->modificado = false;

        if (se_reemplazo_pagina == false){
            log_info(logger, "Reemplazo pagina hecho (pid=%d) (numero de pagina reemplazada=%d) (numero de pagina actual=%d) (numero de marco=%d)", tabla_pointer->pid, numero_pagina_a_reemplazar, numero_pagina_solicitada, numero_marco);
            tabla_pointer->tamanio_conjunto_residente += 1;
        }
        else{
            log_info(logger, "Se trajo pagina a memoria (pid=%d) (numero de pagina=%d) (numero de marco=%d)", tabla_pointer->pid, numero_pagina_solicitada, numero_marco);
        }
    }
    else{ //si la pagina ya esta en memoria
        numero_marco = entrada_segundo_nivel_pointer->numero_marco;
    }

    return numero_marco;
}

entrada_segundo_nivel* get_entrada_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){
    /*
        Obtiene un puntero la entrada de segundo nivel que corresponde a la pagina numero 'numero_pagina'
        dentro de la tabla 'tabla_pointer'.
    */

    int32_t indice_tabla_segundo_nivel = floor(numero_pagina / ENTRADAS_POR_TABLA);
    
    int32_t indice_entrada_segundo_nivel = numero_pagina % ENTRADAS_POR_TABLA;

    tabla_segundo_nivel* tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, indice_tabla_segundo_nivel);

    entrada_segundo_nivel* entrada_segundo_nivel_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, indice_entrada_segundo_nivel);

    return entrada_segundo_nivel_pointer;

}

int32_t elegir_pagina_para_reemplazar(tabla_primer_nivel* tabla_pointer){
    /*
        Elige una pagina a reemplazar de las que forman parte del conjunto residente.
        Esto ocurre cuando el conjunto residente tiene el maximo tamanio posible y se debe
        trasladar una pagina a disco para traer otra pagina a memoria.
        Devuelve el numero de pagina a reemplazar.
    */

    int32_t pagina_elegida = -1;
    
    if (es_algoritmo_reemplazo_clock() == true){
        pagina_elegida = algoritmo_reemplazo_clock(tabla_pointer);
    }
    else{
        pagina_elegida = algoritmo_reemplazo_clock_mejorado(tabla_pointer);
    }

    //en caso de error
    if (pagina_elegida == -1){
        log_error(logger, "No se encontro una pagina a reemplazar con el algoritmo: %s", ALGORITMO_REEMPLAZO);
    }

    return pagina_elegida;
}

int32_t algoritmo_reemplazo_clock(tabla_primer_nivel* tabla_pointer){

    int32_t pagina_elegida = -1;
    bool se_eligio_pagina = false;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    while(se_eligio_pagina == false){

        // Veo si la pagina apuntada tiene el bit de usado en false

        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, tabla_pointer->puntero_clock);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == true)){
            entrada_segundo_nivel_pointer->usado = false;
        }
        else if ((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)){
            pagina_elegida = tabla_pointer->puntero_clock;
            se_eligio_pagina = true;
        }
        
        algoritmo_reemplazo_actualizar_puntero(tabla_pointer);
        
    }

    return pagina_elegida;
}

int32_t algoritmo_reemplazo_clock_mejorado(tabla_primer_nivel* tabla_pointer){

    int32_t pagina_elegida = -1;

    if ((pagina_elegida = clock_mejorado_primer_paso(tabla_pointer)) != -1){
        return pagina_elegida;
    }
    else if ((pagina_elegida = clock_mejorado_segundo_paso(tabla_pointer)) != -1){
        return pagina_elegida;
    }
    else if ((pagina_elegida = clock_mejorado_primer_paso(tabla_pointer)) != -1){
        return pagina_elegida;
    }
    else{
        return clock_mejorado_segundo_paso(tabla_pointer);
    }

}

int32_t clock_mejorado_primer_paso(tabla_primer_nivel* tabla_pointer){

    int i = 0;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    bool se_eligio_pagina = false;

    int32_t pagina_elegida = -1;

    // se busca usado = false, modificado = false

    while(se_eligio_pagina == false && i < tabla_pointer->cantidad_paginas){

        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, tabla_pointer->puntero_clock);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)
            &&(entrada_segundo_nivel_pointer->modificado == false)){

            pagina_elegida = tabla_pointer->puntero_clock;
            se_eligio_pagina = true;
        }
        
        algoritmo_reemplazo_actualizar_puntero(tabla_pointer);

        i++;
        
    }

    return pagina_elegida;
}

int32_t clock_mejorado_segundo_paso(tabla_primer_nivel * tabla_pointer){

    int i = 0;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    bool se_eligio_pagina = false;

    int32_t pagina_elegida = -1;

    // se busca usado = false, modificado = true

    while(se_eligio_pagina == false && i < tabla_pointer->cantidad_paginas){

        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, tabla_pointer->puntero_clock);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)
            &&(entrada_segundo_nivel_pointer->modificado == true)){
            
            //se eligio una pagina
            pagina_elegida = tabla_pointer->puntero_clock;
            se_eligio_pagina = true;
        }
        else if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == true)){
            
            //seteo bit de usado en false
            entrada_segundo_nivel_pointer->usado = false;
        }
        
        algoritmo_reemplazo_actualizar_puntero(tabla_pointer);

        //'i' se utiliza para saber cuando se regresa a la posicion inicial del puntero
        i++;
        
    }

    return pagina_elegida;
}

void algoritmo_reemplazo_actualizar_puntero(tabla_primer_nivel* tabla_pointer){

    if (tabla_pointer->puntero_clock + 1 < tabla_pointer->cantidad_paginas){
        tabla_pointer->puntero_clock++;
    }
    else{
        tabla_pointer->puntero_clock = 0;
    }
}

bool es_algoritmo_reemplazo_clock(void){
    return strcmp(ALGORITMO_REEMPLAZO, "CLOCK") == 0;
}

int32_t elegir_marco_libre(tabla_primer_nivel* tabla_pointer){
    
    /*
        Devuelve el numero de un marco libre en el cual colocar una pagina que se va a traer desde disco.
        Elige el primero que vea disponible.
        Retorna -1 en caso de no haber mas memoria disponible.
    */

    int i = 0;
    int tamanio_bitarray = bitarray_get_max_bit(marcos_libres);
    int marco_elegido = -1;

    for(i = 0; i < tamanio_bitarray; i++){
        
        if (bitarray_test_bit(marcos_libres, i) == false){
            marco_elegido = i;
            break;
        }
    }

    return marco_elegido;
}

void acciones_trasladar_pagina_a_disco(int32_t pid, int32_t numero_pagina, int32_t numero_marco){
    /*
        Envia instruccion a swap para trasladar la pagina numero 'numero_pagina' del proceso asociado a 'tabla_pointer' a disco.
        La misma esta ubicada en el marco numero 'numero_marco'.
        Espera que la accion en swap se realize.
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PAGINA_A_DISCO;
    instruccion.pid = pid;
    instruccion.numero_pagina = numero_pagina;
    instruccion.numero_marco = numero_marco;
    instruccion.semaforo_pointer = &semaforo;

    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo);

    sem_destroy(&semaforo);

    return;
}

void acciones_trasladar_pagina_a_memoria(int32_t pid, int32_t numero_pagina, int32_t numero_marco){

    /*
        Envia instruccion a swap para trasladar la pagina numero 'numero_pagina' del proceso asociado a 'tabla_pointer' a memoria
        en el marco numero 'numero_marco'.
        Espera que la accion en swap se realize.
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = TRASLADAR_PAGINA_A_MEMORIA;
    instruccion.pid = pid;
    instruccion.numero_pagina = numero_pagina;
    instruccion.numero_marco = numero_marco;
    instruccion.semaforo_pointer = &semaforo;

    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo);

    sem_destroy(&semaforo);

    return;
}

int32_t acceder_espacio_usuario_lectura(int32_t numero_marco, int32_t desplazamiento){
    //TODO: IMPLEMENTAR

    // LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())
    
    //TODO: PONER BIT DE USADO EN TRUE
    return 0;
}

bool acceder_espacio_usuario_escritura(int32_t numero_marco, int32_t desplazamiento, int32_t valor){
    //TODO: IMPLEMENTAR

    // LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())

    //TODO: PONER BIT DE USADO EN TRUE
    //TODO: PONER BIT DE MODIFICADO EN TRUE
    return false;
}




bool tiene_tabla_primer_nivel(int32_t pid){
    // TODO    
    return false;
}

tabla_primer_nivel* obtener_tabla_con_pid(int32_t pid){
    // TODO
    return NULL;
}

int get_indice_tabla_pointer(t_list* lista, tabla_primer_nivel* tabla_pointer){

    // Devuelve -1 si no se encuentra en la lista

    int i = 0;

    if (list_is_empty(lista)) return -1;

    for (i = 0; i < list_size(lista); i++){

        if (list_get(lista, i) == tabla_pointer) return i;

    }

    return -1;
}

void enviar_instruccion_swap(instruccion_swap instruccion){
    /*
        Reserva memoria para una instruccion y la pushea
        en la cola utilizando los semaforos correspondientes.
    */

    instruccion_swap* instruccion_pointer;

    if ((instruccion_pointer = malloc(sizeof instruccion)) == NULL){
        log_error(logger, "error al hacer malloc en enviar_instruccion_swap()");
        return;
    }
    
    *instruccion_pointer = instruccion;

    pthread_mutex_lock(&mutex_cola_instrucciones_swap);
    queue_push(cola_instrucciones_swap, instruccion_pointer);
    pthread_mutex_unlock(&mutex_cola_instrucciones_swap);

    sem_post(&contador_cola_instrucciones_swap);

}

bool excede_la_tabla(tabla_primer_nivel* tabla_pointer, int32_t indice){

    tabla_segundo_nivel* ultima_tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, tabla_pointer->cantidad_entradas-1);

    return (indice >= (tabla_pointer->cantidad_entradas-1) * ENTRADAS_POR_TABLA + ultima_tabla_segundo_nivel_pointer->cantidad_entradas);
}