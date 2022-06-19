#include "cpu.h"

bool leer_dir_logica(int32_t direccion_logica, int32_t *puntero_valor_leido){
    int32_t direccion_fisica = -1;

    // traducir direccion

    if(!leer_dir_fisica(direccion_fisica, puntero_valor_leido)){
        log_error(logger, "Error en el acceso a memoria");
        return false;
    }
    log_info(logger, "Leido en memoria: %d", *puntero_valor_leido);

    return true;
}



bool escribir_dir_logica(int32_t direccion_logica, int32_t valor){

    // traducir direccion

    return true;
}

bool leer_dir_fisica(int32_t direccion_fisica, int32_t *puntero_valor_leido){

    // acceder a memoria

    return true;
}

bool escribir_dir_fisica(int32_t direccion_fisica, int32_t *puntero_valor_leido){

    // acceder a memoria 

    return true;
}

// Busca el marco en la memoria. RETORNA MARCO -1 SI HAY UN ERROR EN LA COMUNICACION CON LA MEMORIA
tlb_entrada_t buscar_pagina(int32_t pagina){ 
    dir_logica_t direcccion_logica;
    tlb_entrada_t nueva_entrada;
    int32_t tabla_segundo_nivel;


    if(!acceder_a_tabla_1_nivel(calcular_entrada_primer_nivel(pagina), &tabla_segundo_nivel)){
        nueva_entrada.marco = -1;
        return nueva_entrada;
    }

    // obtener el marco de la memoria
    
    nueva_entrada.pagina = pagina;
    return nueva_entrada;
}


// bool acceder_a_tabla_1_nivel(int32_t indice_primer_nivel, int32_t *tabla_segundo_nivel){

//     /*  Se lo pide a memoria. Le pasa:
//         en_ejecucion.tabla_paginas
//         indice_primer_nivel

//         Memoria devuelve el indice de la tabla de segundo nivel.
//         Lo retorno en la variable tabla_segundo_nivel
//     */

//     return true;
// }


// bool acceder_a_tabla_2_nivel(

// }

/////// VERSION MODIFICADA PARA TESTEAR ///////
int32_t tlb_get_marco(tlb_t tlb, int32_t pagina){//busca en la tlb o en memoria el marco necesario//----creo que ok
    tlb_entrada_t entrada;
    log_info(logger, "Se produjo un fallo de pagina, se buscara en memoria la pagina %d", pagina);
    entrada = buscar_pagina(pagina);//solicito en memoria la pagina que me falta//TODO salvaguardar
    return entrada.marco;
}