#include "kernel.h"

//TODO: implementar semaforos en todos los planificadores que utilicen variables globales
//TODO: implementar semaforos, si hace falta, cuando se llama a un planificador que sea llamado por varias funciones

bool es_algoritmo_srt(void){
    return strcmp(ALGORITMO_PLANIFICACION, "SRT") == 0;
}

void ingresar_proceso_a_ready(int signal){
    /*
        Escucha la signal SIGUSR1. Intenta ingresar un proceso a ready.
        La signal se activa cuando:
        - Entra un nuevo proceso a NEW
        - Entra un proceso a READY_SUSPENDIDO
        - Se suspende un proceso (Entra a BLOQUEADO_SUSPENDIDO)
        - Proceso se va a EXIT
    */

    pthread_mutex_lock(&mutex_grado_multiprogramacion_actual);

    if (grado_multiprogramacion_actual < GRADO_MULTIPROGRAMACION){

        //primero intento con los procesos de READY_SUSPENDIDO porque tienen prioridad
        if (transicion_ready_suspendido_ready()){
            grado_multiprogramacion_actual++;
            si_es_necesario_enviar_interrupcion_o_ready_ejec();
        } //luego con procesos en NEW
        else if(transicion_new_ready()){
            grado_multiprogramacion_actual++;
            si_es_necesario_enviar_interrupcion_o_ready_ejec();
        } //si no se cargo ninguno en READY
        else{
            si_es_necesario_ready_ejec();
        }

    }
    else{
        log_info(logger, "El grado de multiprogramacion actual es el maximo posible");
    }

    pthread_mutex_unlock(&mutex_grado_multiprogramacion_actual);
}

void invocar_ingresar_proceso_a_ready(void){
    kill(getpid(), SIGUSR1);
}

void si_es_necesario_enviar_interrupcion_o_ready_ejec(void){
    
    pthread_mutex_lock(&mutex_en_ejecucion);
    pthread_mutex_lock(&mutex_lista_ready);

    if (es_algoritmo_srt() && en_ejecucion != NULL) enviar_interrupcion_cpu();
    else if (en_ejecucion == NULL) transicion_ready_ejec();
    
    pthread_mutex_unlock(&mutex_en_ejecucion);
    pthread_mutex_unlock(&mutex_lista_ready);
}

void si_es_necesario_ready_ejec(void){
    
    //Si no hay ninguno ejecutandose y hay al menos un proceso en READY entonces mando a EJEC

    pthread_mutex_lock(&mutex_en_ejecucion);
    pthread_mutex_lock(&mutex_lista_ready);

    if (en_ejecucion == NULL && list_size(lista_ready) > 0) transicion_ready_ejec();
    
    pthread_mutex_unlock(&mutex_en_ejecucion);
    pthread_mutex_unlock(&mutex_lista_ready);
}

void transicion_consola_new(char *lista_instrucciones, int32_t tamanio_proceso, int socket){
    /*
        Gestiona la transicion CONSOLA->NEW
    */

    pcb_t *pcb_pointer = generar_pcb(lista_instrucciones, tamanio_proceso, socket);

    pthread_mutex_lock(&mutex_cola_new);
    queue_push(cola_new, pcb_pointer);
    pthread_mutex_unlock(&mutex_cola_new);

    log_info(logger, "-> NEW (PID = %d)", pcb_pointer->pid);

    invocar_ingresar_proceso_a_ready();
}

void transicion_ejec_ready(int32_t tiempo_ejecucion){
    /*
        Gestiona la transicion EJEC->READY
    */

    pthread_mutex_lock(&mutex_en_ejecucion);
    pthread_mutex_lock(&mutex_lista_ready);

    if (en_ejecucion == NULL){
        log_error(logger, "error en transicion_ejec_ready(): 'en_ejecucion' es NULL");
        return;
    }
    
    if (es_algoritmo_srt()) sumar_duracion_rafaga(en_ejecucion, tiempo_ejecucion);

    list_add(lista_ready, en_ejecucion);

    log_info(logger, "EJEC->READY (pid = %d)", en_ejecucion->pid);

    en_ejecucion = NULL;

    transicion_ready_ejec();

    pthread_mutex_unlock(&mutex_en_ejecucion);
    pthread_mutex_unlock(&mutex_lista_ready);
}

int seleccionar_proceso_menor_estimacion(void){
    /*
        Recorre la lista de ready comparando las estimaciones de rafagas
        y devuelve la posicion en la lista de ready del pcb del proceso con menor estimacion.
    */

    // Aclaracion: esta funcion no utiliza semaforos porque ya los utiliza transicion_ready_ejec()
    // Y es la unica funcion que llama a esta

    pcb_t * pcb_actual = NULL;
    pcb_t * pcb_menor_rafaga = NULL;
    int indice_menor_rafaga = 0;
    int i = 0;

    if (!list_is_empty(lista_ready)){
        pcb_menor_rafaga = list_get(lista_ready, 0);
    }

    for (i = 1; i < list_size(lista_ready); i++){

        pcb_actual = list_get(lista_ready, i);

        if ((pcb_actual->estimacion_rafaga - pcb_actual->duracion_real_ultima_rafaga) < ( pcb_menor_rafaga->estimacion_rafaga - pcb_menor_rafaga->duracion_real_ultima_rafaga)){
            pcb_menor_rafaga = pcb_actual;
            indice_menor_rafaga = i;
        }

    }

    return indice_menor_rafaga;
}

void transicion_ready_ejec(void){
    /*
        Gestiona la transicion READY->EJEC
    */

    // Aclaracion: esta funcion no utiliza semaforos porque ya los utilizan las dos funciones
    // que llaman a esta: transicion_ejec_ready() y si_es_necesario_enviar_interrupcion_o_ready_ejec()

    if (en_ejecucion != NULL){
        log_error(logger, "error en transicion_ready_ejec(): 'en_ejecucion' no es NULL");
        return;
    }

    if (list_size(lista_ready) == 0){
        log_info(logger, "No hay procesos en READY para llevar a EJEC");
        return;
    }

    if (es_algoritmo_srt()){
        en_ejecucion = list_remove(lista_ready, seleccionar_proceso_menor_estimacion());
    }
    else{//si es fifo
        en_ejecucion = list_remove(lista_ready, 0);
    } 

    enviar_pcb_cpu(en_ejecucion);

    log_info(logger, "READY->EJEC (PID = %d)", en_ejecucion->pid);

    pthread_mutex_lock(&mutex_interrupcion_cpu);

    if (ya_se_envio_interrupcion_cpu == true){
        ya_se_envio_interrupcion_cpu = false;
    }

    pthread_mutex_unlock(&mutex_interrupcion_cpu);

}

bool transicion_new_ready(void){
    /*
        Gestiona la transicion NEW->READY.
        Devuelve true en caso de haber hecho la transicion, false en caso contrario.
    */
    
    pcb_t *pcb_pointer;

    bool resultado = false;
    
    pthread_mutex_lock(&mutex_cola_new);
    pthread_mutex_lock(&mutex_lista_ready);
    
    if (!queue_is_empty(cola_new)){
        pcb_pointer = queue_peek(cola_new);

        inicializar_estructuras_memoria(pcb_pointer);

        list_add(lista_ready, queue_pop(cola_new));

        log_info(logger, "NEW->READY (PID = %d)", pcb_pointer->pid);

        resultado = true;
    }
    
    pthread_mutex_unlock(&mutex_cola_new);
    pthread_mutex_unlock(&mutex_lista_ready);
    
    return resultado;
}

void transicion_ejec_exit(void){
    /*
        Gestiona la transicion EJEC->EXIT
    */

    pthread_mutex_lock(&mutex_en_ejecucion);
    pthread_mutex_lock(&mutex_grado_multiprogramacion_actual);
    
    if (en_ejecucion == NULL){
        log_error(logger, "error en transicion_ejec_exit(): 'en_ejecucion' es NULL");
        return;
    }

    memoria_finalizar_proceso(en_ejecucion);

    log_info(logger, "EJEC->EXIT (PID=%d)", en_ejecucion->pid);

    finalizar_conexion_consola(en_ejecucion);

    liberar_memoria_pcb(en_ejecucion);
        
    en_ejecucion = NULL;
    
    grado_multiprogramacion_actual--;

    pthread_mutex_unlock(&mutex_en_ejecucion);
    pthread_mutex_unlock(&mutex_grado_multiprogramacion_actual);

    invocar_ingresar_proceso_a_ready();
}

void transicion_ejec_bloqueado(int32_t tiempo_bloqueo, int32_t tiempo_ejecucion){
    /*
        Gestiona la transicion EJEC->BLOQUEADO
    */

    datos_tiempo_bloqueo *datos_sobre_IO = NULL;

    pthread_t hilo;

    pthread_mutex_lock(&mutex_en_ejecucion);

    //calculo nueva estimacion porque ya termino la rafaga
    if (es_algoritmo_srt()){
        sumar_duracion_rafaga(en_ejecucion, tiempo_ejecucion);
        en_ejecucion->estimacion_rafaga = calcular_estimacion_rafaga(en_ejecucion);
        en_ejecucion->duracion_real_ultima_rafaga = 0;
    }

    if ((datos_sobre_IO = malloc(sizeof *datos_sobre_IO)) == NULL){
        log_error(logger, "Error al hacer malloc() en transicion_ejec_bloqueado()");
        return;
    }

    datos_sobre_IO->tiempo_bloqueo = tiempo_bloqueo;
    datos_sobre_IO->pcb_pointer = en_ejecucion;

    list_add(lista_bloqueado, en_ejecucion);

    log_info(logger, "EJEC -> BLOQUEADO (PID = %d)", en_ejecucion->pid);

    en_ejecucion = NULL;

    pthread_mutex_unlock(&mutex_en_ejecucion);

    si_es_necesario_enviar_interrupcion_o_ready_ejec();

    // Inicializo el timer por si despues hace falta suspender el proceso
    pthread_create(&hilo, NULL, hilo_timer, datos_sobre_IO->pcb_pointer);

    pthread_detach(hilo);

    // Pusheo los datos necesarios (tiempo de bloqueo y pcb_pointer) para hacer IO

    pthread_mutex_lock(&mutex_cola_IO);

    queue_push(cola_IO, datos_sobre_IO);

    pthread_mutex_unlock(&mutex_cola_IO);

    // Veo si puedo entrar un proceso a IO

    si_se_puede_entrar_IO();

}

void* hilo_timer(void* pcb_pointer_void){

    /*
        Este hilo espera TIEMPO_MAXIMO_BLOQUEADO milisegundos.
        Si el proceso asociado sigue en BLOQUEADO entonces se pasa a BLOQUEADO_SUSPENDIDO.
        Puede pasar que cuando termine el usleep el proceso ya haya terminado IO y este en READY
    */

    pcb_t *pcb_pointer = pcb_pointer_void;

    usleep(milisegundos_a_microsegundos(TIEMPO_MAXIMO_BLOQUEADO));

    pthread_mutex_lock(&mutex_lista_bloqueado);

    //si el proceso esta en BLOQUEADO
    if (get_indice_pcb_pointer(lista_bloqueado, pcb_pointer) != -1){
        transicion_bloqueado_bloqueado_suspendido(pcb_pointer);
    }

    pthread_mutex_unlock(&mutex_lista_bloqueado);
    
    return NULL;
}

void si_se_puede_entrar_IO(void){
    /*
        Busca en la cola cola_IO y si hay procesos esperando entrar en IO y ademas
        IO no esta ocupada entonces lo entra
    */

    datos_tiempo_bloqueo datos_sobre_IO, *datos_pointer;

    pthread_mutex_lock(&mutex_cola_IO);
    pthread_mutex_lock(&mutex_en_IO);

    if(!queue_is_empty(cola_IO) && en_IO.pcb_pointer == NULL){

        datos_pointer = queue_pop(cola_IO);
        datos_sobre_IO = *datos_pointer;

        free(datos_pointer);

        entrar_IO(datos_sobre_IO.pcb_pointer, datos_sobre_IO.tiempo_bloqueo);
    }

    pthread_mutex_unlock(&mutex_cola_IO);
    pthread_mutex_unlock(&mutex_en_IO);
}

void entrar_IO(pcb_t *pcb_pointer, int32_t tiempo_bloqueo){

    //ACLARACION: no espera un mutex de en_IO porque ya lo hace si_se_puede_entrar_IO() y
    //esta es la unica funcion que la llama

    pthread_t hilo;

    if (en_IO.pcb_pointer != NULL){
        log_error(logger, "Se intento entrar en IO un proceso cuando ya hay uno");
        return;
    }

    //cargo datos necesarios en en_IO
    en_IO.pcb_pointer = pcb_pointer;
    en_IO.tiempo_bloqueo = tiempo_bloqueo;

    //inicializo el timer para esperar el tiempo asignado de IO
    pthread_create(&hilo, NULL, hilo_timer_IO, NULL);

    pthread_detach(hilo);

    log_debug(logger, "-> I/O (PID = %d)", pcb_pointer->pid);

}

void* hilo_timer_IO(void* arg){

    /*    
        Hilo que espera el tiempo de bloqueo asignado. Cuando
        termina la espera llama a terminar_IO
    */

    pthread_mutex_lock(&mutex_en_IO);

    usleep(milisegundos_a_microsegundos(en_IO.tiempo_bloqueo));

    pthread_mutex_unlock(&mutex_en_IO);

    terminar_IO();

    return NULL;
}

void terminar_IO(void){
    /*
        Saca al proceso de IO. Lo mueve a READY o a READY_SUSPENDIDO segun corresponda.
        Trata de traer otro proceso a IO, si es que hay uno
    */

    pthread_mutex_lock(&mutex_en_IO);
    pthread_mutex_lock(&mutex_lista_bloqueado);
    pthread_mutex_lock(&mutex_lista_bloqueado_suspendido);

    if (en_IO.pcb_pointer == NULL){
        log_error(logger, "Se intento terminar de IO un proceso cuando no hay uno");
        return;
    }

    int indice_bloqueado = get_indice_pcb_pointer(lista_bloqueado, en_IO.pcb_pointer);
    int indice_bloqueado_suspendido = 0;

    //si el pcb esta en lista_bloqueado
    if ( indice_bloqueado != -1){
        transicion_bloqueado_ready(en_IO.pcb_pointer);
    }//si esta en lista_bloqueado_suspendido
    else if((indice_bloqueado_suspendido = get_indice_pcb_pointer(lista_bloqueado_suspendido, en_IO.pcb_pointer)) != -1){
        transicion_bloqueado_suspendido_ready_suspendido(en_IO.pcb_pointer);
    }//si no esta en ninguna de las dos (error)
    else{
        log_error(logger, "Error en terminar_IO(): el pcb no esta ni en BLOQUEADO ni en BLOQUEADO_SUSPENDIDO");
        return;
    }

    log_debug(logger, "I/O -> (PID = %d)", en_IO.pcb_pointer->pid);

    en_IO.pcb_pointer = NULL;

    pthread_mutex_unlock(&mutex_en_IO);
    pthread_mutex_unlock(&mutex_lista_bloqueado);
    pthread_mutex_unlock(&mutex_lista_bloqueado_suspendido);

    si_se_puede_entrar_IO();

}

void transicion_bloqueado_bloqueado_suspendido(pcb_t *pcb_pointer){
    /*
        Gestiona la transicion BLOQUEADO->BLOQUEADO_SUSPENDIDO.
        Suspende un proceso
    */

    //ACLARACION: no espera un mutex de la lista de bloqueado porque ya lo hace hilo_timer
    //y es la unica funcion que llama a esta

    memoria_suspender_proceso(pcb_pointer);

    pthread_mutex_lock(&mutex_lista_bloqueado_suspendido);
    pthread_mutex_lock(&mutex_grado_multiprogramacion_actual);

    list_add(lista_bloqueado_suspendido, list_remove(lista_bloqueado, get_indice_pcb_pointer(lista_bloqueado, pcb_pointer)));

    log_info(logger, "BLOQUEADO -> BLOQUEADO_SUSPENDIDO (PID = %d)", pcb_pointer->pid);

    grado_multiprogramacion_actual--;

    pthread_mutex_unlock(&mutex_lista_bloqueado_suspendido);
    pthread_mutex_unlock(&mutex_grado_multiprogramacion_actual);

    invocar_ingresar_proceso_a_ready(); 
}

void transicion_bloqueado_suspendido_ready_suspendido(pcb_t* pcb_pointer){
    /*
        Gestiona la transicion BLOQUEADO_SUSPENDIDO->READY_SUSPENDIDO
        Cuando un proceso suspendido cumplio su tiempo de I/O
    */

    //ACLARACION: no espera un mutex de lista_bloqueado_suspendido
    //porque ya lo hace terminar_IO() y es la unica funcion que llama a esta

    pthread_mutex_lock(&mutex_cola_ready_suspendido);

    queue_push(cola_ready_suspendido, list_remove(lista_bloqueado_suspendido, get_indice_pcb_pointer(lista_bloqueado_suspendido, pcb_pointer)));

    log_info(logger, "BLOQUEADO_SUSPENDIDO -> READY_SUSPENDIDO (PID = %d)", pcb_pointer->pid);

    pthread_mutex_unlock(&mutex_cola_ready_suspendido);
    
    invocar_ingresar_proceso_a_ready();
}

void transicion_bloqueado_ready(pcb_t* pcb_pointer){
    /*
        Gestiona la transicion BLOQUEADO->READY
        Cuando un proceso termina de esperar por I/O sin llegar a ser suspendido
    */

    //ACLARACION: no espera un mutex de lista_bloqueado ni de lista_bloqueado_suspendido
    //porque ya lo hace terminar_IO() y es la unica funcion que llama a esta

    list_add(lista_ready, list_remove(lista_bloqueado, get_indice_pcb_pointer(lista_bloqueado, pcb_pointer)));

    log_info(logger, "BLOQUEADO -> READY (PID = %d)", pcb_pointer->pid);

    si_es_necesario_enviar_interrupcion_o_ready_ejec();
}

bool transicion_ready_suspendido_ready(void){
    /*
        Gestiona la transicion READY_SUSPENDIDO -> READY.
        Devuelve true en caso de haber hecho la transicion, false en caso contrario.
    */

    bool resultado = false;
    pcb_t * pcb_pointer = NULL;

    pthread_mutex_lock(&mutex_cola_ready_suspendido);
    pthread_mutex_lock(&mutex_lista_ready);

    if (!queue_is_empty(cola_ready_suspendido)){

        pcb_pointer = queue_pop(cola_ready_suspendido);

        list_add(lista_ready, pcb_pointer);

        log_info(logger, "READY_SUSPENDIDO->READY (PID = %d)", pcb_pointer->pid);

        resultado = true;
    
    }

    pthread_mutex_unlock(&mutex_cola_ready_suspendido);
    pthread_mutex_unlock(&mutex_lista_ready);

    return resultado;
}

int get_indice_pcb_pointer(t_list* lista, pcb_t* pcb_pointer){

    // Devuelve -1 si no se encuentra en la lista

    int i = 0;

    if (list_is_empty(lista)) return -1;

    for (i = 0; i < list_size(lista); i++){

        if (list_get(lista, i) == pcb_pointer) return i;

    }

    return -1;
}

int32_t milisegundos_a_microsegundos(int32_t milisegundos){
    return milisegundos * 1000;
}