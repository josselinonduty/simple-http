#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "cli.h"

static struct option cli_longopts[6] = {
	{"config", optional_argument, 0, 'c'},
	{"directory", optional_argument, 0, 'd'},
	{"origin", optional_argument, 0, 'o'},
	{"port", optional_argument, 0, 'p'},
	{"max-connections", optional_argument, 0, 'm'},
	{0, 0, 0, 0},
};

static char *cli_shortopts = "c:d:o:p:m:";

typedef struct seen_t {
	int memsize;
	char *seen;
	int size;
} seen_t;

cli_error cli_seen_create(seen_t *seen)
{
	seen->memsize = 16;
	seen->seen = (char *)malloc(seen->memsize);
	if (seen->seen == NULL)
		return cli_args_memory;

	seen->size = 0;
	return cli_ok;
}

cli_error cli_seen_put(seen_t *seen, char c)
{
	if (seen->size == seen->memsize) {
		seen->memsize += 16;
		seen->seen = (char *)realloc(seen->seen, seen->memsize);

		if (seen->seen == NULL)
			return cli_args_memory;
	}

	seen->seen[seen->size] = c;
	seen->size++;
	return cli_ok;
}

cli_error cli_seen_has(seen_t *seen, char c)
{
	for (int i = 0; i < seen->size; i++) {
		if (seen->seen[i] == c)
			return cli_args_duplicate;
	}

	return cli_ok;
}

cli_error cli_seen_ensure_unique(seen_t *seen, char c)
{
	if (cli_seen_has(seen, c) == cli_args_duplicate)
		return cli_args_duplicate;

	cli_seen_put(seen, c);
	return cli_ok;
}

cli_error cli_config_reset(config *config)
{
	config->allow = 0;
	config->port = 80;
	config->vroot = NULL;
	config->max_connections = SOMAXCONN;
	return cli_ok;
}

int cli_config_is_default(config *config)
{
	return config->vroot == NULL;
}

cli_error cli_config_check(config *config)
{
	if (config->port < 0 || config->port > 65535) {
		fprintf(stderr, "Error: Invalid port number\n");
		return cli_config_error;
	}

	if (config->allow < 0) {
		fprintf(stderr, "Error: Invalid allowed ip address\n");
		return cli_config_error;
	}

	if (config->vroot == NULL) {
		fprintf(stderr, "Error: Missing directory\n");
		return cli_config_error;
	}

	if (config->max_connections < 1 || config->max_connections > SOMAXCONN) {
		fprintf(stderr, "Error: Invalid max connections\n");
		return cli_config_error;
	}

	return cli_ok;
}

cli_error cli_load_from_conf(const char *path, config *config)
{
	cli_config_reset(config);

	FILE *fconf = fopen(path, "r");
	if (fconf == NULL)
		return cli_config_file_error;

	// TODO: Parse configuration file
	// config->allow = 1;
	// config->port = 8080;
	// config->vroot = "./";

	fclose(fconf);
	return cli_ok;
}

cli_error cli_load_from_args(int argc, char **argv, config *config)
{
	cli_config_reset(config);

	int option_index = 0;
	seen_t seen;

	cli_error cli_err = cli_seen_create(&seen);
	if (cli_err != cli_ok)
		return cli_err;

	while (1) {
		int c = getopt_long(argc, argv, cli_shortopts, cli_longopts,
				    &option_index);

		if (c == -1)
			break;

		cli_err = cli_seen_ensure_unique(&seen, c);
		if (cli_err != cli_ok) {
			switch (cli_err) {
			case cli_args_duplicate:
				fprintf(stderr,
					"Error: Duplicate option '%c'\n", c);
				break;

			case cli_args_memory:
				fprintf(stderr, "Error: Memory error\n");
				break;

			default:
				fprintf(stderr, "Error: Unknown error\n");
				break;
			}
			return cli_err;
		}

		char *endptr;
		switch (c) {
		case 'c':
			cli_err = cli_load_from_conf(optarg, config);
			if (cli_err != cli_ok)
				return cli_err;
			break;

		case 'p':
			;
			endptr = NULL;
			config->port = strtoul(optarg, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr,
					"Error: Invalid port number '%s'\n",
					optarg);
				return cli_conversion_error;
			}
			break;

		case 'd':
			config->vroot = optarg;
			break;

		case 'o':
			if (inet_pton(AF_INET, optarg, &(config->allow)) != 1) {
				fprintf(stderr,
					"Error: Invalid ip address '%s'\n",
					optarg);
				return cli_conversion_error;
			}
			break;

		case 'm':
			;
			endptr = NULL;
			config->max_connections = strtoul(optarg, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr,
					"Error: Invalid max connections '%s'\n",
					optarg);
				return cli_conversion_error;
			}
			break;

		default:
			fprintf(stderr, "Warning: Unknown option\n");
			break;
		}
	}

	return cli_ok;
}

/**
 * -------- Steps --------
 * 1. Parse command line arguments
 * 2. Try to load from -c <path>
 * 3. Try to load command line arguments
 * 5. Try to load from /usr/local/etc/simple-http.conf
 * 4. Try to load from /etc/simple-http.conf
 * 6. Explode.
 * -----------------------
 */
cli_error cli_load(int argc, char **argv, config *config)
{
	cli_error cli_err;

	cli_config_reset(config);

	if (argc > 1) {
		cli_err = cli_load_from_args(argc, argv, config);

		if (cli_err != cli_ok)
			return cli_err;
	}

	if (cli_config_is_default(config)) {
		cli_err =
		    cli_load_from_conf("/usr/local/etc/simple-http.conf",
				       config);

		if (cli_err == cli_config_file_malformed) {
			fprintf(stderr,
				"Error: Failed to load from configuration file (%s)\n",
				"/usr/local/etc/simple-http.conf");
			return cli_err;
		}
		if (cli_err == cli_ok) {
			printf("Configuration loaded from %s\n",
			       "/usr/local/etc/simple-http.conf");
		}
	}

	if (cli_config_is_default(config)) {
		cli_err = cli_load_from_conf("/etc/simple-http.conf", config);

		if (cli_err == cli_config_file_malformed) {
			fprintf(stderr,
				"Error: Failed to load from configuration file (%s)\n",
				"/etc/simple-http.conf");
			return cli_err;
		}
		if (cli_err == cli_ok) {
			printf("Configuration loaded from %s\n",
			       "/etc/simple-http.conf");
		}
	}

	if (cli_config_is_default(config)) {
		fprintf(stderr,
			"Error: No configuration file was found (tried /etc/simple-http.conf and /usr/local/etc/simple-http.conf)\n");
		return cli_config_file_error;
	}

	cli_err = cli_config_check(config);
	return cli_err;
}
