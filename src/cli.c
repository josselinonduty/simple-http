#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "conf.h"
#include "multiset.h"

static struct option cli_longopts[6] = {
	{"config", optional_argument, 0, 'c'},
	{"directory", optional_argument, 0, 'd'},
	{"origin", optional_argument, 0, 'o'},
	{"port", optional_argument, 0, 'p'},
	{"max-connections", optional_argument, 0, 'm'},
	{0, 0, 0, 0},
};

static char *cli_shortopts = "c:d:o:p:m:";

cli_error cli_config_reset(config *config)
{
	config->allow = 0;
	config->port = 8080;
	config->vroot = "./www";
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

	conf_error conf_err;
	conf_err = conf_load(path, config);
	if (conf_err != CONF_OK) {
		return cli_config_file_malformed;
	}

	return cli_ok;
}

cli_error cli_load_from_args(int argc, char **argv, config *config)
{
	cli_config_reset(config);

	char_multiset_t uset;
	multiset_error uset_err;

	uset_err = char_multiset_create(&uset);
	if (uset_err < 0)
		return uset_err;

	int option_index = 0;
	while (1) {
		int c = getopt_long(argc, argv, cli_shortopts, cli_longopts,
				    &option_index);

		if (c == -1)
			break;

		uset_err = char_multiset_put(&uset, c);
		if (uset_err != MULTISET_OK) {
			switch (uset_err) {
			case MULTISET_DUPLICATE_ERROR:
				fprintf(stderr,
					"Error: Duplicate option '%c'\n", c);
				break;

			case MULTISET_MEMORY_ERROR:
				fprintf(stderr, "Error: Memory error\n");
				break;

			default:
				fprintf(stderr, "Error: Unknown error\n");
				break;
			}
			return uset_err;
		}

		cli_error cli_err;
		char *endptr;
		switch (c) {
		case 'c':
			cli_err = cli_load_from_conf(optarg, config);
			return cli_err;

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
