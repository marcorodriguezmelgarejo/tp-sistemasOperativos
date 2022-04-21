#include <stdio.h>
#include <commons/log.h>
#include <stdlib.h>

#include "../../shared/include/sockets.h"

#define CONSOLA_BACKLOG 5
#define SUCCESS_STATUS 0
#define ERROR_STATUS 1

t_log* crear_logger();