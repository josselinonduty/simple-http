#include <arpa/inet.h>
#include <stdio.h>

#include "cli.h"

int main(int argc, char *argv[])
{
	cli_error cli_err;

	config config;
	cli_err = cli_load(argc, argv, &config);
	if (cli_err != cli_ok) {
		fprintf(stderr, "Error: Failed to load configuration\n");
		return 1;
	}

	printf("Configuration loaded:\n");
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &config.allow, ip, INET_ADDRSTRLEN);
	printf("Allow: %s\n", ip);
	printf("Port: %d\n", config.port);
	printf("Dir: %s\n", config.vroot);


	return 0;
}
