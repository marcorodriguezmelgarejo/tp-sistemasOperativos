#include "cpu.h"

// Retorna el marco de la MP donde esta esa pagina. Busca en la TLB, y si no lo encuentra, busca en memoria y actualiza la TLB.
int32_t buscar_pagina(int32_t numero_pagina){//busca en la tlb o en memoria el marco necesario
    int32_t marco;

    log_info(logger, "Buscando el marco de la pagina %d", numero_pagina);

    if(esta_pagina_en_tlb(numero_pagina)){ // Hit
        marco = obtener_de_tlb(numero_pagina);
        log_info(logger, "TLB hit. Marco obtenido de tlb");
        log_info(logger, "Marco obtenido: %d", marco);
    }else{ // Miss
        log_warning(logger, "TLB miss. Buscando marco en las tablas de paginas");
        marco = buscar_pagina_en_memoria(numero_pagina);
        if(marco == -1){
           log_error(logger, "Error en la comunicacion con memoria, no se pudo obtener el numero de marco");
           return -1;
        }
        log_info(logger, "Marco obtenido: %d. Agregando a TLB.", marco);
        agregar_a_tlb(numero_pagina, marco);
    }

    if(REEEMPLAZO_TLB == LRU) {
        mover_pagina_al_final_de_la_cola(numero_pagina);
    }
    
    
    return marco;
}

void borrar_entrada_TLB_segun_alg(){
    int32_t pagina_elegida;
    pagina_elegida = obtener_elemento_lista_int32(cola_entradas_a_quitar_de_tlb, 0);
    log_info(logger, "TLB llena. Se quitara la pagina %d", pagina_elegida);
    sacar_pagina_de_tlb(pagina_elegida);
}

// Usar solo si ya se sabe que la pagina esta en la tlb (esta_pagina_en_tlb(pag) == true)
int32_t obtener_de_tlb(int32_t pagina){
    char clave[15];
    sprintf(clave, "%d", pagina);
    int32_t *puntero_a_marco = dictionary_get(tlb, clave);
    return *puntero_a_marco;
}

// ---- Modificar la TLB ----
// Escribe (pagina, marco) en tlb, borrando una entrada segun el algoritmo si la tlb esta llena. Si ya hay una entrada con ese marco, la borra.
void agregar_a_tlb(int32_t pagina, int32_t marco){ 

    if(esta_marco_en_tlb(marco)){ // Si ya hay una entrada en la TLB con ese marco, la borra, porque quedo desactualizada.
        log_warning(logger, "Sacando de la tlb la entrada desactualizada del marco %d.", marco);
        sacar_marco_de_tlb(marco);
    }

    if(tlb_esta_llena()){
        borrar_entrada_TLB_segun_alg();
    }
    
    agregar_pagina_al_final_de_la_cola(pagina);

    char clave[15];
    sprintf(clave, "%d", pagina);
    int32_t *puntero_a_marco = malloc(sizeof marco);
    *puntero_a_marco = marco;
    dictionary_put(tlb, clave, puntero_a_marco);

    if(dictionary_size(tlb) != list_size(cola_entradas_a_quitar_de_tlb)){ // No deberia pasar, chequeo por las dudas. Si pasa, hice mal el programa.
        log_error(logger, "Error, la cantidad de entradas en uso de la tlb no coincide con el tamanio de la lista de paginas presentes en la tlb");
        log_error(logger, "Tamanio tlb: %d, tamanio lista de paginas presentes en la tlb: %d", 
            dictionary_size(tlb), list_size(cola_entradas_a_quitar_de_tlb));
    }

    log_debug(logger, "Escrita pagina %d en el marco %d de la tlb", pagina, marco);
}
void sacar_pagina_de_tlb(int32_t pagina){
    // Saca la entrada de tlb que contiene la pagina pasada por parametro
    char clave[15];
    sprintf(clave, "%d", pagina);
    dictionary_remove_and_destroy(tlb, clave, free);

    sacar_pagina_de_la_cola(pagina);

    if(dictionary_size(tlb) != list_size(cola_entradas_a_quitar_de_tlb)){ // No deberia pasar, chequeo por las dudas. Si pasa, hice mal el programa.
        log_error(logger, "Error, la cantidad de entradas en uso de la tlb no coincide con el tamanio de la lista de paginas presentes en la tlb");
        log_error(logger, "Tamanio tlb: %d, tamanio lista de paginas presentes en la tlb: %d", 
            dictionary_size(tlb), list_size(cola_entradas_a_quitar_de_tlb));
    }
}
// ------------------------------------------------------

// ----- MANEJO DE COLA DE PAGINAS -----
// No chequea si la pagina ya esta en la cola. No deberia estar porque solo se usa cuando hay miss de TLB (la pag no esta).
void agregar_pagina_al_final_de_la_cola(int32_t pagina){
    int32_t *puntero_a_numero_pagina = malloc(sizeof pagina);
    *puntero_a_numero_pagina = pagina;
    list_add(cola_entradas_a_quitar_de_tlb, puntero_a_numero_pagina);
}
// Debe la pagina debe estar en la cola
void mover_pagina_al_final_de_la_cola(int32_t pagina){
    sacar_pagina_de_la_cola(pagina);
    agregar_pagina_al_final_de_la_cola(pagina);
}
void sacar_pagina_de_la_cola(int32_t pagina){
    int indice_a_remover = get_indice_lista_int32(cola_entradas_a_quitar_de_tlb, pagina);
    list_remove_and_destroy_element(cola_entradas_a_quitar_de_tlb, indice_a_remover, free);
}
int32_t obtener_elemento_lista_int32(t_list* lista, int32_t indice){
    if(list_size(lista) <= indice){
        log_error(logger, "Se intento obtener el elemento %d de una lista de tamanio %d", indice, list_size(lista));
        return -1;
    }

    int32_t *puntero_elemento_actual;
    puntero_elemento_actual = list_get(lista, indice);
    return *puntero_elemento_actual;
}
int get_indice_lista_int32(t_list* lista, int32_t elemento_buscado){

    // Devuelve -1 si no se encuentra en la lista

    int i = 0;
    int32_t *puntero_elemento_actual;

    if (list_is_empty(lista)){
        return -1;
    }

    for (i = 0; i < list_size(lista); i++){

        puntero_elemento_actual = list_get(lista, i);

        if ((*puntero_elemento_actual) == elemento_buscado){
            return i;
        }
    }

    return -1;
}
// -------------------------------------

bool tlb_esta_llena(){
    int tamanio_actual = dictionary_size(tlb);
    if(tamanio_actual > ENTRADAS_TLB){
        log_error(logger, "Error, la tlb supera su tamanio"); // No deberia pasar, si pasa esta mal hecho el programa.
        return true;
    }
    return tamanio_actual == ENTRADAS_TLB;
}

// Saca la entrada de tlb que contiene el marco pasado por parametro
void sacar_marco_de_tlb(int32_t marco){
    int32_t pagina_a_sacar = pagina_del_marco_en_tlb(marco);
    sacar_pagina_de_tlb(pagina_a_sacar);
}

bool esta_pagina_en_tlb(int32_t pagina){
    char clave[15];
    sprintf(clave, "%d", pagina);
    return dictionary_has_key(tlb, clave);
}

int32_t pagina_del_marco_en_tlb(int32_t marco){
    int tamanio_tlb = dictionary_size(tlb);
    int32_t pagina;

    for(int i = 0; i<tamanio_tlb; i++){
        pagina = obtener_elemento_lista_int32(cola_entradas_a_quitar_de_tlb, i);

        if(obtener_de_tlb(pagina) == marco){
            return pagina;
        }
    }

    return -1;
}

bool esta_marco_en_tlb(int32_t marco){
    return pagina_del_marco_en_tlb(marco) != -1;
}

void inicializar_tlb(){
    tlb = dictionary_create();
    cola_entradas_a_quitar_de_tlb = list_create();
}

void destruir_tlb(){
    dictionary_destroy_and_destroy_elements(tlb, free);
    list_destroy_and_destroy_elements(cola_entradas_a_quitar_de_tlb, free);
}

void vaciar_tlb(){
    dictionary_clean_and_destroy_elements(tlb, free);
    list_destroy_and_destroy_elements(cola_entradas_a_quitar_de_tlb, free);
    cola_entradas_a_quitar_de_tlb = list_create();
}

void si_cambio_el_proceso_vaciar_tlb(pcb_t pcb){
    if(pcb.pid != pid_anterior){
        vaciar_tlb();
    }
}

void prueba_agregar_y_sacar_de_tlb(){
    agregar_a_tlb(0,1000);
    agregar_a_tlb(1,1000);
    agregar_a_tlb(2,2000);

    sacar_pagina_de_tlb(2);

    log_error(logger, "TAMANIO DICCIONARIO: %d", dictionary_size(tlb));
}