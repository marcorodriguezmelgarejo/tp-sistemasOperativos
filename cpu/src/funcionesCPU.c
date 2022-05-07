#include "cpu.h"

t_log* crear_logger(){

	t_log* logger = NULL;

	if((logger = log_create("./cfg/cpu.log","cpu_log",1,LOG_LEVEL_TRACE)) == NULL){
		puts("No se ha podido crear el archivo de log.\nTerminando ejecucion.");
		exit(ERROR_STATUS);
	}

	log_debug(logger, "Se ha creado el archivo de log con exito.");

	return logger;
}

void leer_string_config(t_config* config, char* value, char* return_value, t_log* logger){
	
	if (config_has_property(config, value)){
		strcpy(return_value, config_get_string_value(config, value));
		log_info(logger, "%s: %s", value, return_value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		log_destroy(logger);
		config_destroy(config);
		exit(ERROR_STATUS);	
	}
}

void leer_int_config(t_config* config, char* value, int return_value, t_log* logger){
	if (config_has_property(config, value)){
		return_value = config_get_int_value(config, value);
		log_info(logger, "%s: %d", value, return_value);
	} else {
		log_error(logger, "No se encontro el valor de %s. Finalizando ejecucion.", value);
		log_destroy(logger);
		config_destroy(config);
		exit(ERROR_STATUS);	
	}
}

void leer_algoritmo_reemplazo_config(t_config* config, alg_reemplazo_tlb_t return_value, t_log* logger){
	char string_algoritmo[20];
	if (config_has_property(config, "REEMPLAZO_TLB")){
		strcpy(string_algoritmo, config_get_string_value(config, "REEMPLAZO_TLB"));
		if (!strcmp("FIFO" , string_algoritmo)){
			return_value = FIFO;
			log_info(logger, "REEMPLAZO_TLB: FIFO");
		}
		else{
			if (!strcmp("LRU" , string_algoritmo)){
				return_value = LRU;
				log_info(logger, "REEMPLAZO_TLB: LRU");
			}
			else{
				log_error(logger, "El algoritmo de reemplazo en la config no es valido");
			}
		}
	} else {
		log_error(logger, "No se encontro el valor del algoritmo de reemplazo. Finalizando ejecucion.");
		log_destroy(logger);
		config_destroy(config);
		exit(ERROR_STATUS);	
	}
}

void cargar_config(t_log* logger){

	t_config* config;

	if((config = config_create("./cfg/cpu.config")) == NULL){
		log_error(logger, "No se ha podido leer el archivo de config. \nFinalizando Ejecucion.");
		exit(ERROR_STATUS);	
	}

	leer_string_config(config, "IP_MEMORIA", IP_MEMORIA, logger);
	leer_string_config(config, "PUERTO_MEMORIA", PUERTO_MEMORIA, logger);
	leer_string_config(config, "PUERTO_ESCUCHA_DISPATCH", PUERTO_ESCUCHA_DISPATCH, logger);
	leer_string_config(config, "PUERTO_ESCUCHA_INTERRUPT", PUERTO_ESCUCHA_INTERRUPT, logger);
	leer_int_config(config, "ENTRADAS_TLB", ENTRADAS_TLB, logger);
	leer_int_config(config, "RETARDO_NOOP", RETARDO_NOOP, logger);
	leer_algoritmo_reemplazo_config(config, REEEMPLAZO_TLB, logger);

	log_debug(logger, "Se ha leido el archivo de config con exito.");

	config_destroy(config);
}