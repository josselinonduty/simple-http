#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>

typedef int socket_t;

int socket_create(socket_t *sockd);
int socket_destroy(socket_t *sockd);

#endif