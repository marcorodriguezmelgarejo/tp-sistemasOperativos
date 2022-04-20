
#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>

#include <commons/log.h>
#include <stdbool.h>

#define OK_MESSAGE 0
#define ERROR_MESSAGE 1

// Funciones privadas

void *_sockets_get_in_addr(struct sockaddr *sa);

void _sockets_sigchld_handler(int);

// Funciones publicas

/*
    ACLARACIONES:

    el argumento t_log* puede ser NULL para darle a entender que no hay logger
    
    sockets_enviar_dato() y sockets_recibir_dato() no se pueden usar para structs o strings

    en el argumento data_size se debe pasar el sizeof del tipo de dato a enviar/recibir

    devuelven true en caso de exito, false en error (Utilizar <stdbool.h>)

*/

bool sockets_conectar_como_cliente(char *, char*, int*, t_log*);

bool sockets_abrir_servidor(const char *, int, int*, t_log*);

bool sockets_esperar_cliente(int, int*, t_log*);

bool sockets_enviar_string(int, char *, t_log*);

bool sockets_recibir_string(int, char *, t_log*);

bool sockets_enviar_dato(int, void*, size_t, t_log*);

bool sockets_recibir_dato(int, void*, size_t, t_log*);

void sockets_cerrar(int);

#endif