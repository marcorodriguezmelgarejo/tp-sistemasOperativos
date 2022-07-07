#include "memoria.h"

/*
    TODO: TESTEAR TODAS LAS FUNCIONES

    funciones testeadas:
    inicializar_proceso()
    acceder_tabla_primer_nivel()
    acceder_espacio_usuario_lectura()
    acceder_espacio_usuario_escritura()
*/

tabla_primer_nivel* inicializar_proceso(int32_t pid, int32_t tamanio_proceso){

    if (tamanio_proceso >= TAM_PAGINA * ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA){
        log_error(logger, "El tamanio de proceso excede la cantidad de paginas asignables");
    }
    
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    
    int32_t cantidad_paginas = ceil((float)tamanio_proceso /(float) TAM_PAGINA); //de 1 a ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_primer_nivel = ceil((float)cantidad_paginas /(float) ENTRADAS_POR_TABLA); //de 1 a ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_segundo_nivel_ultima_entrada = cantidad_paginas % ENTRADAS_POR_TABLA; //de 1 a ENTRADAS_POR_TABLA

    enviar_instruccion_swap_CREAR_ARCHIVO_SWAP(pid, tamanio_proceso);
    
    tabla_primer_nivel_pointer = crear_tabla_paginas_proceso(pid, cantidad_paginas, cantidad_entradas_primer_nivel, cantidad_entradas_segundo_nivel_ultima_entrada);

    crear_entrada_diccionario_tabla_pointers(pid, tabla_primer_nivel_pointer);

    log_info(logger, "INICIALIZAR PROCESO (PID = %d) (tamanio de proceso = %d) (cantidad de paginas = %d)", pid, tamanio_proceso, cantidad_paginas);
    
    return tabla_primer_nivel_pointer;
}

void suspender_proceso(tabla_primer_nivel* tabla_pointer){

    marcar_marcos_como_libres_proceso(tabla_pointer);

    enviar_instruccion_swap_TRASLADAR_PROCESO_A_DISCO(tabla_pointer);

    limpiar_lista_paginas_cargadas(tabla_pointer);

    log_info(logger, "SUSPENDER PROCESO (PID = %d)", tabla_pointer->pid);

    return;
}

void finalizar_proceso(tabla_primer_nivel* tabla_pointer){

    int32_t pid = tabla_pointer->pid;

    enviar_instruccion_swap_BORRAR_ARCHIVO_SWAP(pid);

    marcar_marcos_como_libres_proceso(tabla_pointer);

    liberar_memoria_tabla_proceso(tabla_pointer);

    borrar_entrada_diccionario_tabla_pointers(pid);

    log_info(logger, "FINALIZAR PROCESO (PID = %d)", pid);

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
        log_error(logger, "El numero de pagina '%d' excede la cantidad de paginas asignadas al proceso", numero_pagina);
        return -1;
    }

    indice_tabla_segundo_nivel = floor(numero_pagina / ENTRADAS_POR_TABLA);

    return indice_tabla_segundo_nivel;
}

int32_t acceder_tabla_segundo_nivel(tabla_primer_nivel* tabla_pointer, int32_t indice_tabla_segundo_nivel, int32_t numero_pagina_solicitada){

    /*
        Retorna el numero de marco en donde se encuentra la pagina solicitada.
        Si la misma no se encuentra en memoria se trae, reemplazando una pagina si es necesario.
    */

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
                acciones_trasladar_pagina_a_disco(tabla_pointer, numero_pagina_a_reemplazar, numero_marco);
            }

            entrada_segundo_nivel_a_reemplazar_pointer->presencia = false;
        }
        else{
            numero_marco = elegir_marco_libre(tabla_pointer);

            if (numero_marco == -1){
                log_error(logger, "No hay mas memoria disponible");
                return -1;
            }
        }

        acciones_trasladar_pagina_a_memoria(tabla_pointer, numero_pagina_solicitada, numero_marco);

        entrada_segundo_nivel_setear_bits_al_traer_a_memoria(entrada_segundo_nivel_pointer, numero_marco);

    }
    else{ //si la pagina ya esta en memoria
        numero_marco = entrada_segundo_nivel_pointer->numero_marco;
    }

    log_info(logger, "ACCESO TABLA SEGUNDO NIVEL (PID = %d, numero de pagina del proceso = %d)", tabla_pointer->pid, numero_pagina_solicitada);

    return numero_marco;
}

int32_t acceder_espacio_usuario_lectura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento){
    // ACLARACION: LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())

    poner_bit_usado_true(tabla_pointer, numero_pagina);

    int32_t * puntero_al_dato = get_puntero_a_entero_de_espacio_usuario(numero_marco, desplazamiento);

    log_info(logger, "LECTURA (PID = %d, numero_pagina = %d, numero_marco = %d, desplazamiento = %d, valor leido = %d)",tabla_pointer->pid, numero_pagina, numero_marco, desplazamiento, *puntero_al_dato);

    return *puntero_al_dato;
}

int32_t* get_puntero_a_entero_de_espacio_usuario(int32_t numero_marco, int32_t desplazamiento){

    char * espacio_usuario_char_pointer = espacio_usuario;

    espacio_usuario_char_pointer += numero_marco*TAM_PAGINA + desplazamiento;

    void * temporal = espacio_usuario_char_pointer;

    int32_t * puntero_al_dato = temporal;

    return puntero_al_dato;
}

bool acceder_espacio_usuario_escritura(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, int32_t desplazamiento, int32_t valor){
    // ACLARACION: LA PAGINA SIEMPRE ESTA EN MEMORIA (LA TRAE acceder_tabla_segundo_nivel())

    poner_bit_usado_true(tabla_pointer, numero_pagina);
    poner_bit_modificado_true(tabla_pointer, numero_pagina);

    int32_t * puntero_al_dato = get_puntero_a_entero_de_espacio_usuario(numero_marco, desplazamiento);

    *puntero_al_dato = valor;

    log_info(logger, "ESCRITURA (PID = %d, numero_pagina = %d, numero_marco = %d, desplazamiento = %d, valor escrito = %d)", tabla_pointer->pid, numero_pagina, numero_marco, desplazamiento, *puntero_al_dato);

    return true;
}

