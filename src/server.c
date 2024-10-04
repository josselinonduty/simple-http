#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "cli.h"
#include "conf.h"
#include "network.h"
#include "server.h"

int server_init(server_t *server)
{
	int err;

	err = socket_create(&(server->socket));
	if (err < 0)
		return err;

	server->server_addr.sin_family = AF_INET;
	server->server_addr.sin_addr.s_addr = server->config.allow;
	server->server_addr.sin_port = htons(server->config.port);

	err = setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 },
			 sizeof(int));
	if (err < 0)
		return err;

	err =
	    bind(server->socket, (struct sockaddr *)&(server->server_addr),
		 sizeof(server->server_addr));
	if (err < 0) {
		if (EACCES == errno) {
			fprintf(stderr,
				"Error: Port %d is a restricted port. Make sure to run as root.\n",
				server->config.port);
		}
		return err;
	}

	err = getsockname
	    (server->socket, (struct sockaddr *)&server->server_addr,
	     &(unsigned int) { sizeof(server->server_addr) }
	);
	if (err < 0)
		return err;
	else
		fprintf(stdout, "Info: Server running on port %d\n",
			ntohs(server->server_addr.sin_port));

	err = listen(server->socket, server->config.max_connections);
	if (err < 0)
		return err;

	return 0;
}

int server_accept_connection(const server_t server, client_t *client)
{
	struct sockaddr_in client_addr;

	int client_socket =
	    accept(server.socket, (struct sockaddr *)&client_addr,
		   (socklen_t *) & (int) { sizeof(client_addr) }
	);
	if (client_socket < 0)
		return client_socket;

	client->client_addr = client_addr;
	client->socket = client_socket;

	return 0;
}

int server_handle_connection(const client_t client)
{
	char *buffer = (char *)malloc(1024);
	int read_size;

	while ((read_size = read(client.socket, buffer, 1024)) > 0) {
		if (strcmp(buffer, "exit\r\n") == 0) {
			break;
		}

		if (write(client.socket, buffer, read_size) < 0) {
			perror("Write failed.");
			exit(EXIT_FAILURE);
		}

		printf("Received: ");
		for (int i = 0; i < read_size; i++) {
			printf("%d ", (int)buffer[i]);
		}
		printf("\n");
	}
	free(buffer);

	return 0;
}

int server_close_connection(const client_t client)
{
	int err = close(client.socket);
	if (err < 0)
		return err;

	return 0;
}

int server_start(server_t *server)
{
	int err;

	err = server_init(server);
	if (err < 0)
		return err;

	signal(SIGCHLD, SIG_IGN);
	while (1) {
		client_t client;

		err = server_accept_connection(*server, &client);
		if (err < 0)
			return err;

		int pid = fork();
		if (pid == 0) {
			err = server_handle_connection(client);
			if (err < 0)
				return err;

			err = server_close_connection(client);
			if (err < 0)
				return err;
			break;	// Child process should exit
		} else {
			err = server_close_connection(client);
			if (err < 0)
				return err;
		}
	}

	return 0;
}

int server_stop(const server_t server)
{
	return close(server.socket);
}
