#include "cpu.h"

int calcular_numero_pagina(int direccion_logica){
   return  floor(direccion_logica / tamanio_pagina);
}
int32_t calcular_entrada_tabla_1er_nivel(int numero_pagina){
   return  floor(numero_pagina / entradas_tabla_paginas);
}
int32_t calcular_entrada_tabla_2do_nivel(int numero_pagina){
   return  numero_pagina % entradas_tabla_paginas;
}
int calcular_desplazamiento(int direccion_logica, int numero_pagina){
   return  direccion_logica - (numero_pagina * tamanio_pagina);
}