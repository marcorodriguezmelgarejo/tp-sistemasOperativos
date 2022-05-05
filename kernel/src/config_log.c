#include "kernel.h"

void crear_logger(void){

	if((logger = log_create(LOG_FILENAME,"kernel_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(ERROR_STATUS);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");
}

void cargar_config(void){
    
    t_config* config;

	if((config = config_create(CONFIG_FILENAME)) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		exit(ERROR_STATUS);
	}

	leer_str_config(config, "IP_MEMORIA", IP_MEMORIA, logger);
	leer_str_config(config, "PUERTO_MEMORIA", PUERTO_MEMORIA, logger);
    leer_str_config(config, "IP_CPU", IP_CPU, logger);
	leer_str_config(config, "PUERTO_CPU_DISPATCH", PUERTO_CPU_DISPATCH, logger);
	leer_str_config(config, "PUERTO_CPU_INTERRUPT", PUERTO_CPU_INTERRUPT, logger);
    leer_str_config(config, "PUERTO_ESCUCHA", PUERTO_ESCUCHA, logger);
	leer_str_config(config, "ALGORITMO_PLANIFICACION", ALGORITMO_PLANIFICACION, logger);
    leer_int_config(config, "ESTIMACION_INICIAL", &ESTIMACION_INICIAL, logger);
	leer_float_config(config, "ALFA", &ALFA, logger);    
    leer_int_config(config, "GRADO_MULTIPROGRAMACION", &GRADO_MULTIPROGRAMACION, logger);
	leer_int_config(config, "TIEMPO_MAXIMO_BLOQUEADO", &TIEMPO_MAXIMO_BLOQUEADO, logger);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
    return;
}

void leer_str_config(t_config* config, char* value, char* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        strcpy(return_value, config_get_string_value(config, value));
	    log_debug(logger, "%s: %s", value, return_value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}
}

void leer_int_config(t_config* config, char* value, int32_t* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        
        *return_value = config_get_int_value(config, value);
		log_debug(logger, "%s: %d", value, *return_value);

	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}   
}

void leer_float_config(t_config* config, char* value, float* return_value, t_log* logger){
    
    if (config_has_property(config, value)){
        
        *return_value = config_get_double_value(config, value);
		log_debug(logger, "%s: %f", value, *return_value);

	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		exit(ERROR_STATUS);
	}   
}