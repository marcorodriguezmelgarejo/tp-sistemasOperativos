#include "memoria.h"

int32_t milisegundos_a_microsegundos(int32_t milisegundos){
    return milisegundos * 1000;
}

void borrar_entrada_diccionario_tabla_pointers(int32_t pid){
    char dictionary_key[MAX_STRING_SIZE];
    
    sprintf(dictionary_key, "%d", pid);

    dictionary_remove(diccionario_tabla_pointers, dictionary_key);
}

void crear_entrada_diccionario_tabla_pointers(int32_t pid, tabla_primer_nivel* tabla_pointer){

    char dictionary_key[MAX_STRING_SIZE];
    
    sprintf(dictionary_key, "%d", pid);

    dictionary_put(diccionario_tabla_pointers, dictionary_key, tabla_pointer);
}