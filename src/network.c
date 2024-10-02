#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "network.h"

Socket socket_create()
{
	Socket sockd;

	if ((sockd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation failed.");
		exit(EXIT_FAILURE);
	}

	return sockd;
}

void socket_destroy(Socket *socket)
{
}
