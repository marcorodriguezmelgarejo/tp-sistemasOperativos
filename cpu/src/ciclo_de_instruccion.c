#include "cpu.h"
// testeado
void instruccion_siguiente(char* retorno){
	char* lista_instrucciones = malloc(strlen(pcb.lista_instrucciones)+1);
	char instruccion[MAX_INSTRUCCION_SIZE];
	strcpy(lista_instrucciones, pcb.lista_instrucciones);

	// empieza a cortar la cadena y guarda la primer instruccion
	strcpy(instruccion, strtok(lista_instrucciones, "\n"));
	
	// si no necesitabamos la primer instruccion, sigue cortando (esta vez mando null porque strtok se acuerda de la cadena que le mandamos)
	for(int i = 0; i < pcb.program_counter; i++){
		strcpy(instruccion, strtok(NULL, "\n"));
	}

	strcpy(retorno, instruccion);
	free(lista_instrucciones);
}
// testeado
void fetch(char* string_instruccion){
	instruccion_siguiente(string_instruccion);
	pcb.program_counter++; // cuando termine el programa, el PC va a quedar igual a la cant de instr (porque la primer instruccion es la 0)
}

instruccion_t decode(char* string_instruccion){
	

}