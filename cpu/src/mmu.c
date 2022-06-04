#include "cpu.h"

int calcular_numero_pagina(int direccion_logica, int tamanio_pagina){
   return  floor(direccion_logica / tamanio_pagina);
}
int calcular_entrada_tabla_1er_nivel(int numero_pagina, int entradas_por_tabla){
   return  floor(numero_pagina / entradas_por_tabla);
}
int calcular_entrada_tabla_2do_nivel(int numero_pagina, int entradas_por_tabla){
   return  numero_pagina mod(entradas_por_tabla);
}
int calcular_desplazamiento(int direccion_logica,int numero_pagina, int tamanio_pagina){
   return  direccion_logica - (numero_pagina * tamanio_pagina);
}
