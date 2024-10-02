#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include "cli.h"
#include "network.h"
#include "server.h"

void server_start(const config config)
{
	Socket socket = socket_create();

	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = config.allow;
	server_addr.sin_port = htons(config.port);

	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 0 }, sizeof(int));

	if (bind(socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
	    0) {
		perror("Bind failed.");
		exit(EXIT_FAILURE);
	}

	if (listen(socket, 5) < 0) {
		perror("Listen failed.");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in client_addr;

	int client = accept(socket, (struct sockaddr *)&client_addr,
			    (socklen_t *) & (int) { sizeof(client_addr) }
	);
	if (client < 0) {
		perror("Accept failed.");
		exit(EXIT_FAILURE);
	}

	if (send(client, "Hello, World!", 14, 0) < 0) {
		perror("Send failed.");
		exit(EXIT_FAILURE);
	}

	if (close(client) < 0) {
		perror("Close failed.");
		exit(EXIT_FAILURE);
	}

	if (close(socket) < 0) {
		perror("Close failed.");
		exit(EXIT_FAILURE);
	}
}

void server_stop(const config config)
{
}
