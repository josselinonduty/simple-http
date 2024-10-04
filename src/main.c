#include <arpa/inet.h>
#include <stdio.h>

#include "cli.h"
#include "conf.h"
#include "server.h"

int main(int argc, char *argv[])
{
	int err;

	config config;
	err = cli_load(argc, argv, &config);
	if (err < 0) {
		fprintf(stderr, "Error: Failed to load configuration\n");
		return err;
	}

	server_t server = {.config = config };
	err = server_start(&server);
	if (err < 0) {
		fprintf(stderr, "Error: Failed to start server\n");
		return err;
	}

	err = server_stop(server);
	if (err < 0) {
		fprintf(stderr, "Error: Failed to stop server\n");
		return err;
	}

	return 0;
}
