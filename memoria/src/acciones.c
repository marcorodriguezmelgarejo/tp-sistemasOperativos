#include "memoria.h"

int32_t inicializar_proceso(int32_t pid, int32_t tamanio_proceso){
    //TODO: IMPLEMENTAR

    if (tamanio_proceso >= TAM_PAGINA * ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA){
        log_error(logger, "El tamanio de proceso excede la cantidad de paginas asignables");
    }

    int32_t cantidad_paginas = floor(tamanio_proceso / TAM_PAGINA); //de 1 a ENTRADAS_POR_TABLA * ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_primer_nivel = floor(cantidad_paginas / ENTRADAS_POR_TABLA); //de 1 a ENTRADAS_POR_TABLA
    int32_t cantidad_entradas_segundo_nivel_ultima_entrada = cantidad_paginas % ENTRADAS_POR_TABLA; //de 1 a ENTRADAS_POR_TABLA

    crear_archivo_swap(pid, tamanio_proceso);
    
    crear_tabla_paginas_proceso(cantidad_entradas_primer_nivel, cantidad_entradas_segundo_nivel_ultima_entrada);

    return;
}

tabla_primer_nivel* crear_tabla_paginas_proceso(cantidad_entradas_primer_nivel, cantidad_entradas_segundo_nivel_ultima_entrada){
    /*
        Reserva memoria para 1 tabla de paginas de primer nivel y para las tablas de segundo nivel necesarias.
        Agrega los elementos necesarios a las listas globales.
    */
    //TODO: IMPLEMENTAR

    int i = 0, j = 0, entradas_a_crear = 0;
    tabla_primer_nivel* tabla_primer_nivel_pointer;
    tabla_segundo_nivel* tabla_segundo_nivel_pointer;
    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    if ((tabla_primer_nivel_pointer = malloc(sizeof *tabla_primer_nivel_pointer)) == NULL){
        log_error(logger, "error al hacer malloc");
        return NULL;
    }

    (*tabla_primer_nivel_pointer).cantidad_entradas = cantidad_entradas_primer_nivel;
    (*tabla_primer_nivel_pointer).lista_de_entradas = list_create();

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

            //TODO: INICIALIZAR VALORES ENTRADAS SEGUNDO NIVEL

            list_add((*tabla_segundo_nivel_pointer).lista_de_entradas, entrada_segundo_nivel_pointer);
        }

    }

    return tabla_primer_nivel_pointer;
}


void suspender_proceso(int32_t pid){
    //TODO: IMPLEMENTAR
    return;
}

void finalizar_proceso(int32_t pid){
    //TODO: IMPLEMENTAR
    return;
}

int32_t acceder_tabla_primer_nivel(int32_t tabla_primer_nivel, int32_t indice){
    //TODO: IMPLEMENTAR

    //se accede a la tabla y al indice correspondiente para obtener el numero de
    return 0;
}

int32_t acceder_tabla_segundo_nivel(int32_t tabla_segundo_nivel, int32_t pagina){
    //TODO: IMPLEMENTAR
    entrada_segundo_nivel entrada= obtener_entrada_segundo_nivel(tabla_segundo_nivel, pagina);
    
    if (!se_encuentra_en_memoria(entrada)){
        cargar_pagina_memoria();
    }

    return entrada.numero_marco;
}

int32_t acceder_espacio_usuario_lectura(int32_t numero_marco, int32_t desplazamiento){
    //TODO: IMPLEMENTAR
    return 0;
}

bool acceder_espacio_usuario_escritura(int32_t numero_marco, int32_t desplazamiento, int32_t valor){
    //TODO: IMPLEMENTAR
    return false;
}