#include <arpa/inet.h>
#include <errno.h>
#include <magic.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "cli.h"
#include "conf.h"
#include "http.h"
#include "network.h"
#include "rfc1945.h"
#include "server.h"

int server_init(server_t *server)
{
	int err;

	err = socket_create(&(server->socket));
	if (err < 0)
		return err;

	server->server_addr.sin_family = AF_INET;
	server->server_addr.sin_addr.s_addr = server->config.host;
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
		fprintf(stderr, "Info: Server running on port %d\n",
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

int server_handle_connection(const server_t server, const client_t client)
{
	fd_set input;
	FD_ZERO(&input);
	FD_SET(client.socket, &input);

	struct timeval timeout;
	timeout.tv_sec = server.config.request_timeout / 1000;
	timeout.tv_usec = (server.config.request_timeout % 1000) * 1000;

	int ready;
	if (server.config.request_timeout > 0)
		ready = select(client.socket + 1, &input, NULL, NULL, &timeout);
	else
		ready = select(client.socket + 1, &input, NULL, NULL, NULL);
	if (ready < 0)
		return ready;

	if (ready == 0)
		// http_send(client, 408, "Request Timeout");   // not in RFC1945
		return 0;

	int err;

	http_request_t request;
	err = http_request_create(client, &request);
	if (err < 0)
		return err;

	http_response_t response;
	http_response_create(&response);

	if (request.major > 1 || (request.major == 1 && request.minor > 1))
		// http_send(client, 505, "HTTP Version Not Supported");   // not in RFC1945
		return 0;

	if (strcmp(request.uri, "/") == 0) {
		http_response_status(&response, 301);
		cimap_set(response.headers, "Location", "/index.html");
		goto send_text;
	}

	if (strstr(request.uri, "..") != NULL) {
		http_response_status(&response, 400);
		goto send_text;
	}

	char vroot_uri[SERVER_BUFFER_SIZE];
	snprintf(vroot_uri, SERVER_BUFFER_SIZE, "%s%s", server.config.vroot,
		 request.uri);

	char *content_type = malloc(SERVER_BUFFER_SIZE);
	if (!content_type)
		return -1;
	err = http_content_get(vroot_uri, &content_type);

	if (HTTP_ENTITY_NOT_FOUND == err) {
		http_response_status(&response, 404);
		http_response_body(&response, STATUS_TEXT_404);
	} else if (err < 0) {
		http_response_status(&response, 500);
	} else {
		cimap_set(response.headers, "Content-Type", content_type);
	}
	free(content_type);

	if (request.method == HTTP_METHOD_POST) {
		http_response_status(&response, 501);
		goto send_text;
	}

	goto send_file;

 send_text:
	http_response_send(client, &request, &response);
	goto cleanup;

 send_file:
	http_response_send_file(client, &request, &response, vroot_uri);
	goto cleanup;

 cleanup:
	http_request_destroy(&request);
	http_response_destroy(&response);

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

	err = http_content_init();
	if (err < 0)
		return err;

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
			err = server_handle_connection(*server, client);
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
	int err;

	err = http_content_free();
	if (err < 0)
		return err;

	return close(server.socket);
}
