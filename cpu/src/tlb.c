#include "kernel.h"

int32_t tlb_get_marco(tlb_t tlb,int32_t pagina){//busca en la tlb o en memoria el marco necesario//----creo que ok

    bool igual_pagina(tlb_entrada_t entrada_a_comparar) {
         return  pagina==entrada_a_comparar.pagina;}
         //devuelve la entrada segun la pagina que le pedimos

t_entrada entrada=list_find(tlb,(void*) igual_pagina);
if (entrada==NULL){
log_error(logger, "Se produjo un fallo de pagina, se buscara en memoria la pagina %d", pagina);
entrada=buscar_pagina(pagina);//solicito en memoria la pagina que me falta//TODO salvaguardar
cargar_en_tlb(tlb,entrada);
}
return entrada.pagina;
}


int cola_paginas_insertar(t_list * cola_paginas,int num_pagina){
        
    bool igual_pagina(int p) {
         return num_pagina==p;}

    list_remove_and_distroy_by_condition(cola_paginas,(void*) igual_pagina);
    //borro la pagina anteriormente cargada, y dejo la pagina como la mas reciente
    //estaria generando  el historial de list_paginas  pero sin repetidos,
    list_add(cola_paginas,num_pagina);
}


//creo que podria usar polimorfismo a travez de cola_paginas , ya que ahi cargo con logica fifo o lru
cargar_en_tlb(tlb_t tlb,tlb_entrada_t entrada){
if(list_size(tlb)==ENTRADAS_TLB){//si la tlb  se lleno ,elimino segun criterio del algoritmo
    bool igual_entrada(tlb_entrada_t entrada_a_borrar) {
         return  list_get(cola_paginas,0)==entrada_a_borrar.pagina;}
         // en el inicio de la lista estaria la entrada mas vieja utilizada (a remplazar)
    
    list_remove_and_distroy_by_condition(tlb,(void*) igual_entrada);
}
list_add(tlb,entrada);
}





