#include "cpu.h"

// Retorna el marco de la MP donde esta esa pagina. Busca en la TLB, y si no lo encuentra, busca en memoria y actualiza la TLB.
int32_t buscar_pagina(int32_t numero_pagina){//busca en la tlb o en memoria el marco necesario
    int32_t marco;

    if(esta_pagina_en_tlb(numero_pagina)){ // Hit
        marco = obtener_de_tlb(numero_pagina);
        log_info(logger, "TLB hit. La pagina %d esta en el marco %d", numero_pagina, marco);
    }else{ // Miss
        marco = buscar_pagina_en_memoria(numero_pagina);
        if(marco == -1){
           log_error(logger, "Error en la comunicacion con memoria, no se pudo obtener el numero de marco");
           return -1;
        }
        log_info(logger, "TLB miss. La pagina %d esta en el marco %d", numero_pagina, marco);
        agregar_a_tlb(numero_pagina, marco);
    }

    if(REEEMPLAZO_TLB == LRU) {
        mover_pagina_al_final_de_la_cola(numero_pagina);
    }
    
    return marco;
}

void borrar_entrada_TLB_segun_alg(){
    int32_t pagina_elegida;
    pagina_elegida = (int32_t) list_get(cola_entradas_a_quitar_de_tlb, 0);
    sacar_pagina_de_tlb(pagina_elegida);
}

// Usar solo si ya se sabe que la pagina esta en la tlb (esta_pagina_en_tlb(pag) == true)
int32_t obtener_de_tlb(int32_t pagina){
    char clave[15];
    sprintf(clave, "%d", pagina);
    return (int32_t) dictionary_get(tlb, clave);
}

// ---- Modificar la TLB ----
// Escribe (pagina, marco) en tlb, borrando una entrada segun el algoritmo si la tlb esta llena. Si ya hay una entrada con ese marco, la borra.
void agregar_a_tlb(int32_t pagina, int32_t marco){ 
    
    if(esta_marco_en_tlb(marco)){ // Si ya hay una entrada en la TLB con ese marco, la borra, porque quedo desactualizada.
        sacar_marco_de_tlb(marco);
    }

    if(tlb_esta_llena()){
        borrar_entrada_TLB_segun_alg();
    }
    
    agregar_pagina_al_final_de_la_cola(pagina);

    char clave[15];
    sprintf(clave, "%d", pagina);
    dictionary_put(tlb, clave, (void*) marco);
}
void sacar_pagina_de_tlb(int32_t pagina){
    // Saca la entrada de tlb que contiene la pagina pasada por parametro
    char clave[15];
    sprintf(clave, "%d", pagina);
    dictionary_remove(tlb, clave);
    sacar_pagina_de_la_cola(pagina);
}
// ------------------------------------------------------

// ----- MANEJO DE COLA DE PAGINAS -----
// No chequea si la pagina ya esta en la cola. No deberia estar porque solo se usa cuando hay miss de TLB (la pag no esta).
void agregar_pagina_al_final_de_la_cola(int32_t pagina){
    list_add(cola_entradas_a_quitar_de_tlb, (void*) pagina);
}
// Debe la pagina debe estar en la cola
void mover_pagina_al_final_de_la_cola(int32_t pagina){
    sacar_pagina_de_la_cola(pagina);
    list_add(cola_entradas_a_quitar_de_tlb, (void*) pagina);
}
void sacar_pagina_de_la_cola(int32_t pagina){
    int indice_a_remover = get_indice_lista_int32(cola_entradas_a_quitar_de_tlb, pagina);
    list_remove(cola_entradas_a_quitar_de_tlb, indice_a_remover);
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
    int ultima_entrada_tlb = dictionary_size(tlb);
    char clave[15];
    int32_t pagina;

    for(int i = 0; i<=ultima_entrada_tlb; i++){
        pagina = (int32_t) list_get(cola_entradas_a_quitar_de_tlb, i);
        sprintf(clave, "%d", pagina);
        if((int32_t) dictionary_get(tlb, clave) == marco){
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
    dictionary_destroy(tlb);
    list_destroy(cola_entradas_a_quitar_de_tlb);
}

void vaciar_tlb(){
    dictionary_clean(tlb);
    list_destroy(cola_entradas_a_quitar_de_tlb);
    cola_entradas_a_quitar_de_tlb = list_create();
}

void si_cambio_el_proceso_vaciar_tlb(pcb_t pcb){
    if(pcb.pid == pid_anterior){
        vaciar_tlb();
    }
}