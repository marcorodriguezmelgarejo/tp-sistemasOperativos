#include "cpu.h"

// Retorna el marco de la MP donde esta esa pagina. RETORNA MARCO -1 SI HAY UN ERROR EN LA COMUNICACION CON LA MEMORIA
int32_t buscar_pagina_en_memoria(int32_t numero_pagina){
    int32_t tabla_segundo_nivel;
    int32_t marco;

    if(!acceder_a_tabla_1_nivel(numero_pagina, &tabla_segundo_nivel)){
        log_error(logger, "Error al acceder a la tabla de 1er nivel. Pid: %d, Indice: %d", en_ejecucion.pid, calcular_entrada_tabla_1er_nivel(numero_pagina));
        return -1;
    }

    if(!acceder_a_tabla_2_nivel(tabla_segundo_nivel, numero_pagina, &marco)){
        log_error(logger, "Error al acceder a la tabla de 2do nivel. Pid: %d, Num tabla 2do nivel: %d, Indice: %d", en_ejecucion.pid, tabla_segundo_nivel, calcular_entrada_tabla_2do_nivel(numero_pagina));
        return -1;
    }
    
    return marco;
}

bool leer_dir_logica(int32_t direccion_logica, int32_t *puntero_valor_leido){
    int32_t num_pagina = calcular_numero_pagina(direccion_logica);
    int32_t desplazamiento = calcular_desplazamiento(direccion_logica, num_pagina);
    int32_t marco = buscar_pagina(num_pagina);
    if(marco == -1){
        log_error(logger, "Error al obtener el marco. No se ejecutara la operacion.");
        return false;
    }

    if(!leer_dir_fisica(num_pagina, marco, desplazamiento, puntero_valor_leido)){
        log_error(logger, "Error en el acceso a memoria");
        return false;
    }
    log_info(logger, "Leido en memoria: %d", *puntero_valor_leido);

    return true;
}

bool escribir_dir_logica(int32_t direccion_logica, int32_t valor){
    int32_t num_pagina = calcular_numero_pagina(direccion_logica);
    int32_t desplazamiento = calcular_desplazamiento(direccion_logica, num_pagina);
    int32_t marco = buscar_pagina(num_pagina);
    if(marco == -1){
        log_error(logger, "Error al obtener el marco. No se ejecutara la operacion.");
        return false;
    }

    if(!escribir_dir_fisica(num_pagina, marco, desplazamiento, valor)){
        log_error(logger, "Error en el acceso a memoria");
        return false;
    }
    log_info(logger, "Valor escrito en memoria");

    return true;
}

bool leer_dir_fisica(int32_t num_pag, int32_t marco, int32_t desplazamiento, int32_t *puntero_valor_leido){
    int32_t motivo = LECTURA_EN_ESPACIO_USUARIO;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof pid, logger)
        &&
        sockets_enviar_dato(memoria_socket, &num_pag, sizeof num_pag, logger)
        &&
        sockets_enviar_dato(memoria_socket, &marco, sizeof marco, logger)
        &&
        sockets_enviar_dato(memoria_socket, &desplazamiento, sizeof desplazamiento, logger)
        &&
        sockets_recibir_dato(memoria_socket, puntero_valor_leido, sizeof(*puntero_valor_leido), logger)
    );
    

}

bool escribir_dir_fisica(int32_t num_pag, int32_t marco, int32_t desplazamiento, int32_t dato){
    char respuesta_memoria[10];
    int32_t motivo = ESCRITURA_EN_ESPACIO_USUARIO;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof pid, logger)
        &&
        sockets_enviar_dato(memoria_socket, &num_pag, sizeof num_pag, logger)
        &&
        sockets_enviar_dato(memoria_socket, &marco, sizeof marco, logger)
        &&
        sockets_enviar_dato(memoria_socket, &desplazamiento, sizeof desplazamiento, logger)
        &&
        sockets_enviar_dato(memoria_socket, &dato, sizeof(dato), logger)
        &&
        sockets_recibir_string(memoria_socket, respuesta_memoria, logger)
        &&
        (strcmp(respuesta_memoria, "OK") == 0)
    );
}

bool acceder_a_tabla_1_nivel(int32_t numero_pagina, int32_t *tabla_segundo_nivel){
    int32_t motivo = ACCESO_TABLA_PRIMER_NIVEL;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof(pid), logger)
        &&
        sockets_enviar_dato(memoria_socket, &numero_pagina, sizeof(numero_pagina), logger)
        &&
        sockets_recibir_dato(memoria_socket, tabla_segundo_nivel, sizeof(*tabla_segundo_nivel), logger)
    );
}

bool acceder_a_tabla_2_nivel(int32_t tabla_segundo_nivel, int32_t numero_pagina, int32_t *marco){
    int32_t motivo = ACCESO_TABLA_SEGUNDO_NIVEL;
    int32_t pid = en_ejecucion.pid;

    return(
        sockets_enviar_dato(memoria_socket, &motivo, sizeof motivo, logger)
        &&
        sockets_enviar_dato(memoria_socket, &pid, sizeof(pid), logger)
        &&
        sockets_enviar_dato(memoria_socket, &tabla_segundo_nivel, sizeof(tabla_segundo_nivel), logger)
        &&
        sockets_enviar_dato(memoria_socket, &numero_pagina, sizeof(numero_pagina), logger)
        &&
        sockets_recibir_dato(memoria_socket, marco, sizeof(*marco), logger)
    );
}

