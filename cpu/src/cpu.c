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

int main(){     // prueba de decode()
    crear_logger();

    char* string_instruccion = malloc(MAX_INSTRUCCION_SIZE);
    int tamanio_instruccion = MAX_INSTRUCCION_SIZE;
    instruccion_t instruccion;
    operacion_t operacion;

    while(1){
        getline(&string_instruccion, &tamanio_instruccion, stdin);
        instruccion = decode(string_instruccion);
        printf("Codigo operacion: %d\n", instruccion.operacion);
       	switch(instruccion.operacion){
            case INVALIDA:
                break;
	    	case NO_OP:
			    break;
		    case EXIT:
			    break;
		    case I_O:
                printf("Tiempo bloqueo: %d\n", instruccion.tiempo_bloqueo);
			    break;
		    case READ:
                printf("Dir origen: %d\n", instruccion.dir_origen);
			    break;
		    case WRITE:
                printf("Dir destino: %d\n", instruccion.dir_destino);
	    		break;
    		case COPY:
                printf("Dir destino: %d\n", instruccion.dir_destino);
                printf("Dir origen: %d\n", instruccion.dir_origen);
			    break;
	    }
    }
    free(string_instruccion);
    return 0;
} 
