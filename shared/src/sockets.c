#include "sockets.h"

void *_sockets_get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool sockets_conectar_como_cliente(char *ip, char* port, int* sockfd_pointer ,t_log* logger)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        if (logger != NULL) log_error(logger, "getaddrinfo: %s", gai_strerror(rv));
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
        freeaddrinfo(servinfo);
        return false;
    }

    inet_ntop(p->ai_family, _sockets_get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo);
    
    if (logger != NULL)log_info(logger, "Conectandose a IP: %s en puerto: %s", s, port);

    *sockfd_pointer = sockfd;

    return true;
}

void _sockets_sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

bool sockets_abrir_servidor(const char *port, int backlog, int* sockfd_pointer, t_log* logger)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        if (logger != NULL) log_error(logger, "getaddrinfo: %s", gai_strerror(rv));
        return false;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            if (logger != NULL)log_error(logger, "Error al abrir servidor: %s", strerror(errno));
            return false;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        if (logger != NULL)log_error(logger, "Error al hacer bind: %s", strerror(errno));
            return false;
    }

    if (listen(sockfd, backlog) == -1) {
        if (logger != NULL)log_error(logger, "Error al hacer listen: %s", strerror(errno));
            return false;
    }

    *sockfd_pointer = sockfd;

    sa.sa_handler = _sockets_sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        if (logger != NULL)log_error(logger, "Error en sigaction(): %s", strerror(errno));
        return false;
    }

    if (logger != NULL)log_info(logger, "Escuchando conexiones entrantes en puerto %s...", port);

    return true;
}

bool sockets_esperar_cliente(int sockfd, int* new_fd_pointer, t_log* logger){

    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size = sizeof their_addr;
    int new_fd = 0;

    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1) {
        if (logger != NULL) log_error(logger, "Error en accept(): %s", strerror(errno));
        return false;
    }

    inet_ntop(their_addr.ss_family, _sockets_get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

    if (logger != NULL) log_info(logger, "Conexion aceptada desde %s", s);

    *new_fd_pointer = new_fd;

    return true;
}

bool sockets_enviar_string(int sockfd, char * str, t_log* logger){
    
    //los primeros 4 bytes (int32_t) corresponden al largo de la string
    
	int numbytes = 0;

    unsigned int buffer_size = strlen(str) + sizeof(int32_t);

    void * buffer = malloc(buffer_size);

    if (buffer == NULL){
        if (logger != NULL)log_error(logger, "Error al asignar memoria al buffer");
        return false;
    }
    
    int32_t length_str = strlen(str);

	memcpy(buffer, &length_str, sizeof(int32_t));
    memcpy(buffer + sizeof(int32_t), str, strlen(str));

   	if ( ( numbytes = send(sockfd, buffer, buffer_size, 0) ) == -1 )
    {
        if (logger != NULL)log_error(logger, "Error al enviar string: %s", strerror(errno));
        return false;
    }

    free(buffer);

    return true;

}

bool sockets_recibir_string(int sockfd, char * buffer, t_log* logger){

    int32_t buflen = 0;

    int numbytes = 0;

    if ((numbytes = recv( sockfd , &buflen, sizeof(int32_t),  MSG_WAITALL )) == -1  )//recibe largo de string
    {
        if (logger != NULL)log_error(logger, "Error al recibir string: %s", strerror(errno));
        return false;
    }

    if ((numbytes = recv(sockfd, buffer, buflen, MSG_WAITALL)) == -1  )//recibo el string
    {
        if (logger != NULL)log_error(logger, "Error al recibir string: %s", strerror(errno));
        return false;
    }

    buffer[numbytes] = '\0';

    return true;
}

bool sockets_enviar_dato(int sockfd, void* data_pointer, size_t data_size, t_log* logger){
    
    //IMPORTANTE: no utilizar para struct ni string

    int numbytes = 0;

    if ( ( numbytes = send(sockfd, data_pointer, data_size, 0) ) == -1 )
    {
        if (logger != NULL)log_error(logger, "Error al enviar dato: %s", strerror(errno));
        return false;
    }

    return true;

}

bool sockets_recibir_dato(int sockfd, void* data_pointer, size_t data_size, t_log* logger){

    //IMPORTANTE: no utilizar para struct ni string

    int numbytes = 0;

    if ( ( numbytes = recv(sockfd, data_pointer, data_size, MSG_WAITALL) ) == -1 )
    {
        if (logger != NULL)log_error(logger, "Error al recibir dato: %s", strerror(errno));
        return false;
    }

    return true;

}

void sockets_cerrar(int sockfd){
    close(sockfd);
}
