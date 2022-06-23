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
    int32_t motivo = LECTURA_EN_ESPACIO_USUARIO;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &direccion_fisica, sizeof(direccion_fisica), logger) // esto o el marco y el desp por separado?
        &&
        sockets_recibir_dato(memoria_socket, puntero_valor_leido, sizeof(*puntero_valor_leido), logger)
    );
    

}

bool escribir_dir_fisica(int32_t direccion_fisica, int32_t dato){
    char respuesta_memoria[10];
    int32_t motivo = ESCRITURA_EN_ESPACIO_USUARIO;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &direccion_fisica, sizeof(direccion_fisica), logger) // esto o el marco y el desp por separado?
        &&
        sockets_enviar_dato(memoria_socket, &dato, sizeof(dato), logger)
        &&
        sockets_recibir_string(memoria_socket, respuesta_memoria, logger)
        &&
        strcmp(respuesta_memoria, "OK")
    );
}

// Busca el marco en la memoria. RETORNA MARCO -1 SI HAY UN ERROR EN LA COMUNICACION CON LA MEMORIA
tlb_entrada_t buscar_pagina(int32_t numPagina){ 
    int32_t tabla_segundo_nivel;
    tlb_entrada_t nueva_entrada;
    nueva_entrada.pagina = numPagina;

    if(!acceder_a_tabla_1_nivel(calcular_entrada_tabla_1er_nivel(numPagina), &tabla_segundo_nivel)){
        log_error(logger, "Error al acceder a la tabla de 1er nivel. Pid: %d, Indice: %d", en_ejecucion.pid, calcular_entrada_tabla_1er_nivel(numPagina));
        nueva_entrada.marco = -1;
        return nueva_entrada;
    }

    if(!acceder_a_tabla_2_nivel(tabla_segundo_nivel, calcular_entrada_tabla_2do_nivel(numPagina), &(nueva_entrada.marco))){
        log_error(logger, "Error al acceder a la tabla de 2do nivel. Pid: %d, Num tabla 2do nivel: %d, Indice: %d", en_ejecucion.pid, tabla_segundo_nivel, calcular_entrada_tabla_2do_nivel(numPagina));
        nueva_entrada.marco = -1;
        return nueva_entrada;
    }
    
    return nueva_entrada;
}


bool acceder_a_tabla_1_nivel(int32_t indice_primer_nivel, int32_t *tabla_segundo_nivel){
    int32_t motivo = ACCESO_TABLA_PRIMER_NIVEL;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof(pid), logger)
        &&
        sockets_enviar_dato(memoria_socket, &indice_primer_nivel, sizeof(indice_primer_nivel), logger)
        &&
        sockets_recibir_dato(memoria_socket, tabla_segundo_nivel, sizeof(*tabla_segundo_nivel), logger)
    );
}


bool acceder_a_tabla_2_nivel(int32_t tabla_segundo_nivel, int32_t indice_segundo_nivel, int32_t *marco){
    int32_t motivo = ACCESO_TABLA_SEGUNDO_NIVEL;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof(pid), logger)
        &&
        sockets_enviar_dato(memoria_socket, &tabla_segundo_nivel, sizeof(tabla_segundo_nivel), logger)
        &&
        sockets_enviar_dato(memoria_socket, &indice_segundo_nivel, sizeof(indice_segundo_nivel), logger)
        &&
        sockets_recibir_dato(memoria_socket, marco, sizeof(*marco), logger)
    );
}

/////// VERSION MODIFICADA PARA TESTEAR ///////
int32_t tlb_get_marco(tlb_t tlb, int32_t numPagina){//busca en la tlb o en memoria el marco necesario//----creo que ok
    tlb_entrada_t entrada;
    log_info(logger, "Se produjo un fallo de numPagina, se buscara en memoria la numPagina %d", numPagina);
    entrada = buscar_pagina(numPagina);//solicito en memoria la numPagina que me falta//TODO salvaguardar
    return entrada.marco;
}