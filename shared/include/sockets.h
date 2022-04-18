#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <commons/log.h>
#include <stdbool.h>

void *_get_in_addr(struct sockaddr *);

bool conectar_socket_como_cliente(int, char*, int*, t_log*);

bool enviar_string(int, char *, t_log*);

bool recibir_string(int, char *, t_log*);

void cerrar_socket(int);

#define MAXDATASIZE 100 // maxima cantidad de bytes que se pueden leer de una llamada