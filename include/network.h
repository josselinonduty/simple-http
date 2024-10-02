#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>

typedef int Socket;

Socket socket_create(void);
void socket_destroy(Socket *socket);

#endif