#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <stdbool.h>

#include "cli.h"
#include "network.h"

typedef struct server_t {
    struct sockaddr_in server_addr;
    socket_t socket;
    config config;
} server_t;

typedef struct client_t {
    struct sockaddr_in client_addr;
    socket_t socket;
} client_t;

int server_start(server_t *server);
int server_stop(const server_t server);

#endif