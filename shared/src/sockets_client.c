#include "sockets.h"

// get sockaddr, IPv4 or IPv6:
void *_get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool conectar_socket_como_cliente(int port, char* ip, int* sockfd ,t_log* logger)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        if (logger != NULL) log_error(logger, "getaddrinfo: %s\n", gai_strerror(rv));
        return false;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        if (logger != NULL) log_error(logger, "Fallo al conectar al servidor");
        return false;
    }

    inet_ntop(p->ai_family, _get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo);
    
    if (logger != NULL)log_info(logger, "Conectandose a %s\n", s);

    return true;
}

bool enviar_string(int sockfd, char * str, t_log* logger){

	int aux = 0, numbytes = 0;

	aux = strlen(str);

    if ( ( numbytes = send(new_fd, &aux, sizeof(int), 0) ) == -1 )//la cantidad de datos a enviar en el proximo send
    {
        if (logger != NULL)log_error(logger, "Error al enviar string: %s\n", strerror(errno));
        return false;
    }

   	if ( ( numbytes = send(new_fd, str, strlen(str), 0) ) == -1 )//envio la string
    {
        if (logger != NULL)log_error(logger, "Error al enviar string: %s\n", strerror(errno));
        return false;
    }

    return true;

}

bool recibir_string(int sockfd, char * buf, t_log* logger){

    int buflen = 0, numbytes = 0;

    if ((numbytes = recv( sockfd , &buflen, sizeof(int),  0 )) == -1  )//la cantidad de datos a recibir en el proximo recv
    {
        if (logger != NULL)log_error(logger, "Error al recibir string: %s\n", strerror(errno));
        return false;
    }

    if ((numbytes = recv( sockfd , buf , buflen,   0 )) == -1  )//recibo el string
    {
        if (logger != NULL)log_error(logger, "Error al recibir string: %s\n", strerror(errno));
        return false;
    }

    buf[numbytes] = '\0';

    return true;
}

void cerrar_socket(int sockfd){
    close(sockfd);
}