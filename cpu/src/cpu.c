#include "sockets.h"
#include "prueba.h"
#include "cpu.h"

int main(){
    t_log* logger;

    logger = crear_logger();
    cargar_config(logger);
    getchar();
}