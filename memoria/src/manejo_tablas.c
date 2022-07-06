#include "memoria.h"

/*

    Funciones testeadas:
    crear_tabla_paginas_proceso()

*/

tabla_primer_nivel* crear_tabla_paginas_proceso(int32_t pid, int32_t cantidad_paginas, int32_t cantidad_entradas_primer_nivel, int32_t cantidad_entradas_segundo_nivel_ultima_entrada){
    /*
        Reserva memoria para 1 tabla de paginas de primer nivel, para las tablas de segundo nivel
        y para las entradas de segundo nivel.
    */

    int i = 0, j = 0, entradas_a_crear = 0;
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    if ((tabla_primer_nivel_pointer = malloc(sizeof *tabla_primer_nivel_pointer)) == NULL){
        log_error(logger, "error al hacer malloc");
        return NULL;
    }

    inicializar_valores_tabla_primer_nivel(tabla_primer_nivel_pointer, pid, cantidad_paginas, cantidad_entradas_primer_nivel);

    //reservo memoria e inicializo las tablas de segundo nivel
    for (i = 0; i < cantidad_entradas_primer_nivel; i++){

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

        tabla_segundo_nivel_pointer->cantidad_entradas = entradas_a_crear;
        tabla_segundo_nivel_pointer->lista_de_entradas = list_create();

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

            list_add(tabla_segundo_nivel_pointer->lista_de_entradas, entrada_segundo_nivel_pointer);
        }

        list_add(tabla_primer_nivel_pointer->lista_de_tabla_segundo_nivel, tabla_segundo_nivel_pointer);

    }

    return tabla_primer_nivel_pointer;
}

void inicializar_valores_tabla_primer_nivel(tabla_primer_nivel* tabla_primer_nivel_pointer, int32_t pid, int32_t cantidad_paginas, int32_t cantidad_entradas_primer_nivel){
    tabla_primer_nivel_pointer->pid = pid;
    tabla_primer_nivel_pointer->cantidad_entradas = cantidad_entradas_primer_nivel;
    tabla_primer_nivel_pointer->tamanio_conjunto_residente = 0;
    tabla_primer_nivel_pointer->puntero_clock = 0;
    tabla_primer_nivel_pointer->lista_paginas_cargadas = list_create();
    tabla_primer_nivel_pointer->cantidad_paginas = cantidad_paginas;
    tabla_primer_nivel_pointer->lista_de_tabla_segundo_nivel = list_create();
}

void liberar_memoria_tabla_proceso(tabla_primer_nivel* tabla_pointer){
    /*
        Hace todos los frees correspondientes para liberar memoria alocada para una tabla de primer nivel
    */

    int i = 0;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;

    for (i = 0; i < tabla_pointer->cantidad_entradas; i++){
        tabla_segundo_nivel_pointer = list_remove(tabla_pointer->lista_de_tabla_segundo_nivel, 0);
        list_destroy_and_destroy_elements(tabla_segundo_nivel_pointer->lista_de_entradas, free);
    }

    list_destroy_and_destroy_elements(tabla_pointer->lista_paginas_cargadas, free);
    list_destroy(tabla_pointer->lista_de_tabla_segundo_nivel);

    free(tabla_pointer);
}

void poner_bit_usado_true(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){

    entrada_segundo_nivel * entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, numero_pagina);
    
    entrada_segundo_nivel_pointer->usado = true;
}

void poner_bit_modificado_true(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina){

    entrada_segundo_nivel * entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, numero_pagina);
    
    entrada_segundo_nivel_pointer->modificado = true;
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

int get_indice_lista_int32(t_list* lista, int32_t elemento_buscado){

    // Devuelve -1 si no se encuentra en la lista

    int i = 0;
    int32_t *elemento_actual_pointer;

    if (list_is_empty(lista)){
        return -1;
    }

    for (i = 0; i < list_size(lista); i++){

        elemento_actual_pointer = list_get(lista, i);

        if (*elemento_actual_pointer == elemento_buscado){
            return i;
        }
    }

    return -1;
}

bool excede_la_tabla(tabla_primer_nivel* tabla_pointer, int32_t indice){

    tabla_segundo_nivel* ultima_tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, tabla_pointer->cantidad_entradas-1);

    return (indice >= (tabla_pointer->cantidad_entradas-1) * ENTRADAS_POR_TABLA + ultima_tabla_segundo_nivel_pointer->cantidad_entradas);
}

tabla_primer_nivel* obtener_tabla_con_pid(int32_t pid){
    /*
        Busca en el diccionario de tabla pointers el puntero correspondiente al pid.
        Retorna NULL en caso de fallo.
    */

    // Paso el pid a string

    char dictionary_key[MAX_STRING_SIZE];
    
    sprintf(dictionary_key, "%d", pid);

    if (dictionary_has_key(diccionario_tabla_pointers, dictionary_key) == false){
        log_error(logger, "Error en obtener_tabla_con_pid(): no se encuentra la entrada del diccionario correspondiente al pid = %d", pid);
        return NULL;
    }

    return dictionary_get(diccionario_tabla_pointers, dictionary_key);
}

void agregar_pagina_lista_paginas_cargadas(tabla_primer_nivel *tabla_pointer, int32_t numero_pagina){
    int32_t * numero_pagina_pointer = malloc(sizeof numero_pagina);
    *numero_pagina_pointer = numero_pagina;

    list_add(tabla_pointer->lista_paginas_cargadas, numero_pagina_pointer);
}

void quitar_pagina_lista_paginas_cargadas(tabla_primer_nivel *tabla_pointer, int32_t numero_pagina){
    /*
        Remueve la pagina indicada de la lista de paginas cargadas
        y cambia el puntero de clock
    */
    int32_t indice = get_indice_lista_int32(tabla_pointer->lista_paginas_cargadas, numero_pagina);

    int32_t *elemento_pointer = list_remove(tabla_pointer->lista_paginas_cargadas, indice);

    free(elemento_pointer);

    cambiar_puntero_clock(tabla_pointer, indice);
}

void cambiar_puntero_clock(tabla_primer_nivel* tabla_pointer, int32_t indice_quitado){
    
    if (indice_quitado < tabla_pointer->puntero_clock){
        tabla_pointer->puntero_clock -= 1;
    }
    else if(indice_quitado == tabla_pointer->puntero_clock
    && puntero_es_ultimo_elemento_o_excede_lista(tabla_pointer)){
        tabla_pointer->puntero_clock = 0;
    }
    //en caso de que el indice quitado sea mayor al puntero, este queda igual

    //o en caso de que el indice quitado sea igual al puntero pero mientras que este
    //no exceda ni sea el ultimo elemento de la lista, entonces tambien queda igual
}

bool puntero_es_ultimo_elemento_o_excede_lista(tabla_primer_nivel *tabla_pointer){
    int32_t tamanio_lista = list_size(tabla_pointer->lista_paginas_cargadas);

    return ((tabla_pointer->puntero_clock + 1) >= tamanio_lista);
}

void limpiar_lista_paginas_cargadas(tabla_primer_nivel* tabla_pointer){
    list_clean(tabla_pointer->lista_paginas_cargadas);
}

int32_t get_pagina_apuntada_lista_paginas_cargadas(tabla_primer_nivel* tabla_pointer){
    int32_t *numero_pagina_pointer = list_get(tabla_pointer->lista_paginas_cargadas, tabla_pointer->puntero_clock);

    int32_t numero_pagina = *numero_pagina_pointer;

    return numero_pagina;
}

void entrada_segundo_nivel_setear_bits_al_traer_a_memoria(entrada_segundo_nivel* entrada_segundo_nivel_pointer, int32_t numero_marco){
    entrada_segundo_nivel_pointer->presencia = true;
    entrada_segundo_nivel_pointer->numero_marco = numero_marco;
    entrada_segundo_nivel_pointer->usado = true;
    entrada_segundo_nivel_pointer->modificado = false;
}