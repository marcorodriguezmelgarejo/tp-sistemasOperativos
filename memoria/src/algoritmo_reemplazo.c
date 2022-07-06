#include "memoria.h"

int32_t elegir_pagina_para_reemplazar(tabla_primer_nivel* tabla_pointer){
    /*
        Elige una pagina a reemplazar de las que forman parte del conjunto residente.
        Esto ocurre cuando el conjunto residente tiene el maximo tamanio posible y se debe
        trasladar una pagina a disco para traer otra pagina a memoria.
        Devuelve el numero de pagina a reemplazar.
    */

    int32_t pagina_elegida = -1;

    //si funciona bien nunca se deberia entrar en este if
    if(list_size(tabla_pointer->lista_paginas_cargadas) == 0){
        log_error(logger, "No hay paginas para reemplazar (PID = %d)", tabla_pointer->pid);
        return -1;
    }
    
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
    int32_t pagina_actual = 0;
    bool se_eligio_pagina = false;

    entrada_segundo_nivel* entrada_segundo_nivel_pointer;

    while(se_eligio_pagina == false){

        // Veo si la pagina apuntada tiene el bit de usado en false

        pagina_actual = get_pagina_apuntada_lista_paginas_cargadas(tabla_pointer);
        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, pagina_actual);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == true)){
            entrada_segundo_nivel_pointer->usado = false;
        }
        else if ((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)){
            pagina_elegida = pagina_actual;
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
    int32_t pagina_actual;

    // se busca usado = false, modificado = false

    while(se_eligio_pagina == false && i < tabla_pointer->cantidad_paginas){

        pagina_actual = get_pagina_apuntada_lista_paginas_cargadas(tabla_pointer);
        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, pagina_actual);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)
            &&(entrada_segundo_nivel_pointer->modificado == false)){

            pagina_elegida = pagina_actual;
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
    int32_t pagina_actual;

    // se busca usado = false, modificado = true

    while(se_eligio_pagina == false && i < tabla_pointer->cantidad_paginas){

        pagina_actual = get_pagina_apuntada_lista_paginas_cargadas(tabla_pointer);
        entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, pagina_actual);

        if((entrada_segundo_nivel_pointer-> presencia == true) && (entrada_segundo_nivel_pointer->usado == false)
            &&(entrada_segundo_nivel_pointer->modificado == true)){
            
            //se eligio una pagina
            pagina_elegida = pagina_actual;
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

    if (tabla_pointer->puntero_clock + 1 < list_size(tabla_pointer->lista_paginas_cargadas)){
        tabla_pointer->puntero_clock++;
    }
    else{
        tabla_pointer->puntero_clock = 0;
    }
}

bool es_algoritmo_reemplazo_clock(void){
    return strcmp(ALGORITMO_REEMPLAZO, "CLOCK") == 0;
}