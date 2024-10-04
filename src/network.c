#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "network.h"

int socket_create(socket_t *sockd)
{
	int err = socket(AF_INET, SOCK_STREAM, 0);
	if (err < 0)
		return err;

	*sockd = err;
	return 0;
}

int socket_destroy(socket_t *sockd)
{
	int err = close(*sockd);
	if (err < 0)
		return err;

	return 0;
}
