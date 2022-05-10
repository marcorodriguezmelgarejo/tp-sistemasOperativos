#include "sockets.h"
#include "prueba.h"
#include "cpu.h"
#include "protocolos.h"

// int main(){
//     char instruccion_string[MAX_INSTRUCCION_SIZE];
    

//     while(true){
//         recibir_pcb();
//         while(true){
            
//         }

//     }

// }

int main(){
    crear_logger();

    pcb.program_counter = 0;
    pcb.lista_instrucciones = malloc(MAX_INSTRUCCION_SIZE*50+1);
    strcpy(pcb.lista_instrucciones, "I/O 3000\nREAD 0\nWRITE 4 42\nNO_OP\nNO_OP\nCOPY 0 4\nEXIT\n");
    char instruccion_string[MAX_INSTRUCCION_SIZE];

    fetch(instruccion_string);

    log_debug(logger, "Instruccion: %s, PC: %d", instruccion_string, pcb.program_counter);

    fetch(instruccion_string);

    log_debug(logger, "Instruccion: %s, PC: %d", instruccion_string, pcb.program_counter);
}