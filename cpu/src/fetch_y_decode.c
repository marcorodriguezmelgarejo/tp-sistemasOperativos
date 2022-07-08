#include "cpu.h"


void instruccion_siguiente(char* retorno){ // testeado
	char instruccion[MAX_INSTRUCCION_SIZE];
	char* lista_instrucciones;
	int largo_lista_instrucciones;
	int program_counter;

	pthread_mutex_lock(&mutex_PCB);
	largo_lista_instrucciones = strlen(en_ejecucion.lista_instrucciones);
	program_counter = en_ejecucion.program_counter;
	pthread_mutex_unlock(&mutex_PCB);

	lista_instrucciones = malloc(largo_lista_instrucciones + 1);

	pthread_mutex_lock(&mutex_PCB);
	strcpy(lista_instrucciones, en_ejecucion.lista_instrucciones);
	pthread_mutex_unlock(&mutex_PCB);

	// empieza a cortar la cadena y guarda la primer instruccion
	strcpy(instruccion, strtok(lista_instrucciones, "\n"));
	
	// si no necesitabamos la primer instruccion, sigue cortando (esta vez mando null porque strtok se acuerda de la cadena que le mandamos)
	for(int i = 0; i < program_counter; i++){
		strcpy(instruccion, strtok(NULL, "\n"));
	}

	strcpy(retorno, instruccion);
	free(lista_instrucciones);
}

void fetch(char* string_instruccion){
	instruccion_siguiente(string_instruccion);

	pthread_mutex_lock(&mutex_PCB);
	en_ejecucion.program_counter++; // cuando termine el programa, el PC va a quedar igual a la cant de instr (porque la primer instruccion es la 0)
	pthread_mutex_unlock(&mutex_PCB);

	// decrementar las rafagas restantes en srt? (preguntarle a lean)
}

// podemos poner cualquier cantidad de espacios entre los parametros
int primer_parametro(char* line){
	int i = 0;
	int j = 0;
	char primer_parametro[MAX_INSTRUCCION_SIZE];
	while(line[i] != ' '){
		i++;
	}
	// OPERACION| 123 123 <- ESTOY ACA
	while(line[i] == ' '){
		i++;
	}
	// OPERACION |123 123 <- ESTOY ACA
	if(!isdigit(line[i])) {
		log_error(logger, "Falta primer parametro en instruccion %s", line);
	}
	while(isdigit(line[i])){
		primer_parametro[j] = line[i];
		i++;
		j++;
	}
	// OPERACION 123| 123 <- ESTOY ACA
	primer_parametro[j]=0;
	return atoi(primer_parametro);
}

// podemos poner cualquier cantidad de espacios entre los parametros
int segundo_parametro(char* line){
	int i = 0;
	int j = 0;
	char segundo_parametro[MAX_INSTRUCCION_SIZE];
	while(line[i] != ' '){
		i++;
	}
	// OPERACION| 123 123 <- ESTOY ACA
	while(line[i] == ' '){
		i++;
	}
	// OPERACION |123 123 <- ESTOY ACA
	while(isdigit(line[i])){
		i++;
	}
	// OPERACION 123| 123 <- ESTOY ACA
	while(line[i] == ' '){
		i++;
	}
	// OPERACION 123 |123 <- ESTOY ACA
	if(!isdigit(line[i])) {
		log_error(logger, "Falta segundo parametro en instruccion %s", line);
	}
	// Leo y retorno el segundo parametro:
	while(isdigit(line[i])){
		segundo_parametro[j] = line[i];
		i++;
		j++;
	}
	segundo_parametro[j]=0;
	return atoi(segundo_parametro);
}

operacion_t decode_operacion(char* string_instruccion){
	char string_operacion[MAX_INSTRUCCION_SIZE];
	int i = 0;

	//obtiene la representacion en string de la instruccion	
	while(isupper(string_instruccion[i]) || string_instruccion[i] == '_' || string_instruccion[i] == '/'){
		string_operacion[i] = string_instruccion[i];
		i++;
	}
	if(i==0){
		log_error(logger, "Operacion invalida en la instruccion: '%s'", string_instruccion);
		return INVALIDA;
	}
	string_operacion[i] = '\0';

	if(strcmp(string_operacion, "NO_OP") == 0){
		return NO_OP;
	}
	if(strcmp(string_operacion, "I/O") == 0){
		return I_O;
	}
	if(strcmp(string_operacion, "READ") == 0){
		return READ;
	}
	if(strcmp(string_operacion, "WRITE") == 0){
		return WRITE;
	}
	if(strcmp(string_operacion, "COPY") == 0){
		return COPY;
	}
	if(strcmp(string_operacion, "EXIT") == 0){
		return EXIT;
	}
	log_error(logger, "Operacion invalida: %s", string_operacion);
	return INVALIDA;
}

// TODO: que chequee la cant de parametros (como esta ahora si le mandamos mas parametros solo los)
instruccion_t decode(char* string_instruccion){
	instruccion_t instruccion;
	log_info(logger, "---- Comienzo instruccion: '%s' ----", string_instruccion);

	if(string_instruccion[0] == '\0') {
		log_error(logger, "Error instruccion cadena vacia");
		instruccion.operacion = INVALIDA;
	}
	if(string_instruccion[0] == '\n') {
		log_error(logger, "Error instruccion solo con un salto de linea");
		instruccion.operacion = INVALIDA;
	}

	operacion_t operacion = decode_operacion(string_instruccion);
	instruccion.operacion = operacion;

	switch(instruccion.operacion){
		case NO_OP:
			break;
		case EXIT:
			break;
		case I_O:
			instruccion.tiempo_bloqueo = primer_parametro(string_instruccion);
			break;
		case READ:
			instruccion.dir_origen = primer_parametro(string_instruccion);
			break;
		case WRITE:
			instruccion.dir_destino = primer_parametro(string_instruccion);
			instruccion.valor = segundo_parametro(string_instruccion);	
			break;
		case COPY:
			instruccion.dir_destino = primer_parametro(string_instruccion);
			instruccion.dir_origen = segundo_parametro(string_instruccion);
			break;
		case INVALIDA:
			break;
	}

	return instruccion;
}