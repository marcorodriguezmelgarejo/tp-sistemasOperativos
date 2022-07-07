#include "memoria.h"

void iniciar_test_acciones(void){
    tabla_primer_nivel* tabla_pointer = test_inicializar_proceso();

    log_warning(logger, "-----------INICIO TEST LECTURA Y ESCRITURA-----------");

    acceder_espacio_usuario_escritura(tabla_pointer, 1, 1, 8, 1122);
    acceder_espacio_usuario_escritura(tabla_pointer, 1, 1, 12, 4781);

    acceder_espacio_usuario_lectura(tabla_pointer, 1, 1, 8);
    log_warning(logger, "el valor leido debe ser '1122'");

    acceder_espacio_usuario_lectura(tabla_pointer, 1, 1, 12);
    log_warning(logger, "el valor leido debe ser '4781'");

    entrada_segundo_nivel * entrada_segundo_nivel_pointer = get_entrada_segundo_nivel(tabla_pointer, 1);

    if ((entrada_segundo_nivel_pointer->usado == true) && (entrada_segundo_nivel_pointer->modificado == true)){
        log_warning(logger, "los bits de usado y modificado son true, como deberian ser");
    }
    else{
        log_error(logger, "los bits de usado y modificado estan mal");
    }

    log_warning(logger, "-----------FIN TEST LECTURA Y ESCRITURA-----------");

    log_warning(logger, "Saliendo del programa...");

    kill(getpid(), SIGINT);
}

tabla_primer_nivel* test_inicializar_proceso(void){

    int i = 0, j = 0;
    tabla_segundo_nivel * tabla_segundo_nivel_pointer;
    entrada_segundo_nivel * entrada_segundo_nivel_pointer;

    pthread_create(&h3, NULL, hilo_swap, NULL);

    tabla_primer_nivel * tabla_pointer = inicializar_proceso(4, 264);

    //testeo dictionary
    void * temporal = dictionary_get(diccionario_tabla_pointers, "4");

    log_warning(logger, "-----------INICIO TEST INICIALIZAR PROCESO-----------");
    log_warning(logger, "tamanio del diccionario despues de crear la entrada: %d", dictionary_size(diccionario_tabla_pointers));

    log_warning(logger, "tabla_pointer: %p  dictionary_get: %p (tienen que ser iguales)", tabla_pointer, temporal);

    tabla_pointer = temporal;

    //testeo la tabla generada

    log_warning(logger,"pid=%d (tiene que ser '4'), tamanio_lista:%d (tiene que ser '2')", tabla_pointer->pid, list_size(tabla_pointer->lista_de_tabla_segundo_nivel));

    for (i = 0; i < list_size(tabla_pointer->lista_de_tabla_segundo_nivel); i++){
        tabla_segundo_nivel_pointer = list_get(tabla_pointer->lista_de_tabla_segundo_nivel, i);
        log_warning(logger, "tamanio_lista_segundo_nivel: %d (primero tiene que ser '4', luego '1')", list_size(tabla_segundo_nivel_pointer->lista_de_entradas));

        for (j = 0; j < list_size(tabla_segundo_nivel_pointer->lista_de_entradas); j++){
            entrada_segundo_nivel_pointer = list_get(tabla_segundo_nivel_pointer->lista_de_entradas, j);
            log_warning(logger, "numero marco: %d (tiene que ser '-1')", entrada_segundo_nivel_pointer->numero_marco);
        }

    }

    log_warning(logger, "-----------FIN TEST INICIALIZAR PROCESO-----------");
    
    return tabla_pointer;
}

void iniciar_test_algoritmo_reemplazo(void){

    tabla_primer_nivel* tabla_pointer = test_inicializar_proceso();
    log_warning(logger, "-----------INICIO TEST ALGORITMO REEMPLAZO (%s)-----------",ALGORITMO_REEMPLAZO);
    //el puntero clock esta inicializado en cero

    agregar_pagina(tabla_pointer, 1, 0, true);
    agregar_pagina(tabla_pointer, 0, 1, true);
    agregar_pagina(tabla_pointer, 2, 2, false);
    agregar_pagina(tabla_pointer, 4, 3, true);

    log_warning(logger, "pagina elegida para reemplazar: %d (tendria que ser dos)", elegir_pagina_para_reemplazar(tabla_pointer));

    log_warning(logger, "-----------FIN TEST ALGORITMO REEMPLAZO (%s)-----------", ALGORITMO_REEMPLAZO);
}

void agregar_pagina(tabla_primer_nivel* tabla_pointer, int32_t numero_pagina, int32_t numero_marco, bool usado){
    entrada_segundo_nivel* entrada_pointer = get_entrada_segundo_nivel(tabla_pointer, numero_pagina);
    entrada_pointer->presencia = true;
    entrada_pointer->usado = usado;
    entrada_pointer->modificado = true;
    entrada_pointer->numero_marco = numero_marco;
    marcar_marco_como_ocupado(numero_marco);
    agregar_pagina_lista_paginas_cargadas(tabla_pointer, numero_pagina);
}