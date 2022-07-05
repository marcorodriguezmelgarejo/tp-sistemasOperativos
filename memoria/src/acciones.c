#include "memoria.h"

/*
    TODO: TESTEAR TODAS LAS FUNCIONES

    funciones testeadas hasta ahora: ninguna
*/

tabla_primer_nivel* inicializar_proceso(int32_t pid, int32_t tamanio_proceso){

    // TODO: agregar (tabla*, pid) al diccionario

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

void suspender_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Marca marcos utilizados por el proceso como libres 
        y envia instruccion a swap para mandar todo el conjunto residente del proceso a disco
    */

    instruccion_swap instruccion;
    sem_t semaforo;

    marcar_marcos_como_libres_proceso(tabla_pointer);

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
        Borra el archivo de swap del proceso, marca los marcos ocupados por el proceso como libres
        y libera la tabla de primer nivel del proceso
    */

    instruccion_swap instruccion;
    int32_t pid = tabla_pointer->pid;
    sem_t semaforo;

    sem_init(&semaforo, 0, 0);

    instruccion.numero_instruccion = BORRAR_ARCHIVO_SWAP;
    instruccion.pid = tabla_pointer->pid;
    instruccion.semaforo_pointer = &semaforo;
    enviar_instruccion_swap(instruccion);

    sem_wait(&semaforo); //espera a que termine de ejecutarse la instruccion en swap

    sem_destroy(&semaforo);

    marcar_marcos_como_libres_proceso(tabla_pointer);

    liberar_memoria_tabla_proceso(tabla_pointer);

    log_info(logger, "Se finalizo el proceso con PID = %d", pid);

    return;
}

int32_t acceder_tabla_primer_nivel(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){

    /*
        Devuelve el indice de la lista tabla_pointer->lista_de_entradas en donde se encuentre la pagina
    */

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

int32_t acceder_tabla_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t indice_tabla_segundo_nivel, int32_t numero_pagina_solicitada){

    /*
        Retorna el numero de marco en donde se encuentra la pagina solicitada.
        Si la misma no se encuentra en memoria se trae, reemplazando una pagina si es necesario.
    */

    
    bool se_reemplazo_pagina = false;

    int32_t numero_pagina_a_reemplazar;

    tabla_segundo_nivel * tabla_segundo_nivel_pointer;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    entrada_segundo_nivel* entrada_segundo_nivel_a_reemplazar_pointer;

    int32_t numero_marco;
    
    tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, indice_tabla_segundo_nivel);
    entrada_segundo_nivel_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, numero_pagina_solicitada % ENTRADAS_POR_TABLA);

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

        if (se_reemplazo_pagina == true){
            log_info(logger, "Reemplazo pagina hecho (pid=%d) (numero de pagina reemplazada=%d) (numero de pagina actual=%d) (numero de marco=%d)", tabla_pointer->pid, numero_pagina_a_reemplazar, numero_pagina_solicitada, numero_marco);
        }
        else{
            marcar_marco_como_ocupado(numero_marco);
            tabla_pointer->tamanio_conjunto_residente += 1;
            log_info(logger, "Se trajo pagina a memoria (pid=%d) (numero de pagina=%d) (numero de marco=%d)", tabla_pointer->pid, numero_pagina_solicitada, numero_marco);
        }
    }
    else{ //si la pagina ya esta en memoria
        numero_marco = entrada_segundo_nivel_pointer->numero_marco;
    }

    return numero_marco;
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

int32_t acceder_espacio_usuario_lectura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento){
    // ACLARACION: LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())

    poner_bit_usado_true(tabla_pointer, numero_pagina);

    int32_t * puntero_al_dato = get_puntero_a_entero_de_espacio_usuario(numero_marco, desplazamiento);

    log_info(logger, "Lectura en numero_marco = %d, desplazamiento = %d, valor leido = %d", numero_marco, desplazamiento, *puntero_al_dato);

    return *puntero_al_dato;
}

int32_t* get_puntero_a_entero_de_espacio_usuario(int32_t numero_marco, int32_t desplazamiento){

    char * espacio_usuario_char_pointer = espacio_usuario;

    espacio_usuario_char_pointer += numero_marco*TAM_PAGINA + desplazamiento;

    void * temporal = espacio_usuario_char_pointer;

    int32_t * puntero_al_dato = temporal;

    return puntero_al_dato;
    
    //return espacio_usuario_int_pointer[numero_marco*TAM_PAGINA + desplazamiento / tamanio_entero_en_bytes];

    /*
        con tamanio de pagina = 8

        0,0 -> 0
        0,4 -> 4
        1,0 -> 8
    */
}

bool acceder_espacio_usuario_escritura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento, int32_t valor){
    // ACLARACION: LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())

    poner_bit_usado_true(tabla_pointer, numero_pagina);
    poner_bit_modificado_true(tabla_pointer, numero_pagina);

    int32_t * puntero_al_dato = get_puntero_a_entero_de_espacio_usuario(numero_marco, desplazamiento);

    *puntero_al_dato = valor;

    log_info(logger, "Escritura en numero_marco = %d, desplazamiento = %d, valor escrito = %d", numero_marco, desplazamiento, *puntero_al_dato);

    return true;
}

tabla_primer_nivel* obtener_tabla_con_pid(int32_t pid){

    // Buscar en la lista de tablas de primer nivel la tabla que tenga el pid pasado como parametro. 
    // Retornar -1 y logear un error si no se encuentra la tabla (no deberia pasar pero chequear por las dudas).

    return NULL;
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