#include "memoria.h"

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/memoria.log","memoria_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		salir_error(NULL, NULL);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");

	return logger;
}

void cargar_config(){

	if((config = config_create("./cfg/memoria.config")) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		salir_error(logger, NULL);
	}

	leer_config_string(config, "PUERTO_ESCUCHA", PUERTO_ESCUCHA);
	leer_config_int(config, "TAM_MEMORIA", &TAM_MEMORIA);
	leer_config_int(config, "TAM_PAGINA", &TAM_PAGINA);
	leer_config_int(config, "ENTRADAS_POR_TABLA", &ENTRADAS_POR_TABLA);
	leer_config_int(config, "RETARDO_MEMORIA", &RETARDO_MEMORIA);
	leer_config_string(config, "ALGORITMO_REEMPLAZO", ALGORITMO_REEMPLAZO);
	leer_config_int(config, "MARCOS_POR_PROCESO", &MARCOS_POR_PROCESO);
	leer_config_int(config, "RETARDO_SWAP", &RETARDO_SWAP);
	leer_config_string(config, "PATH_SWAP", PATH_SWAP);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
}

void leer_config_string(t_config* config, char* key, char* value){
	
	if (config_has_property(config, key)){
		strcpy(value, config_get_string_value(config, key));
		log_info(logger, "%s: %s", key, value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", key);
		salir_error(logger, NULL);
	}
}

void leer_config_int(t_config* config, char* key, int32_t* value){
	
	if (config_has_property(config, key)){
		*value = config_get_int_value(config, key);
		log_info(logger, "%s: %d", key, *value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", key);
		salir_error(logger, NULL);
	}
}