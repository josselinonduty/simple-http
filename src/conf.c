#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"

#ifndef getline
ssize_t getline(char **lineptr, size_t *memsize, FILE *stream)
{
	*memsize = 64;
	*lineptr = malloc(*memsize);
	if (NULL == *lineptr)
		return -1;

	size_t charsize = 0;	// for null terminator

	char c;
	while ((c = fgetc(stream)) != EOF) {
		if (charsize >= *memsize) {
			*memsize *= 2;
			*lineptr = realloc(*lineptr, *memsize);
			if (NULL == *lineptr)
				return -1;
		}

		if (c == '\n')
			break;
		(*lineptr)[charsize++] = c;
	}

	(*lineptr)[charsize] = '\0';
	return charsize;
}
#endif

const char *conf_regex = "^([a-zA-Z][a-zA-Z_]*)\\s*=\\s*([^=\\s]+)$";

conf_error conf_match_arg(const char *line, char *arg, char *value)
{
	regex_t regex;
	regmatch_t matches[3];
	int err;

	err = regcomp(&regex, conf_regex, REG_EXTENDED);
	if (err != 0) {
		return CONF_MALFORMED_ERROR;
	}

	err = regexec(&regex, line, 3, matches, 0);
	if (err != 0)
		return CONF_MALFORMED_ERROR;

	regfree(&regex);

	if (matches[1].rm_so == -1 || matches[2].rm_so == -1)
		return CONF_MALFORMED_ERROR;

	int arg_len = matches[1].rm_eo - matches[1].rm_so;
	int value_len = matches[2].rm_eo - matches[2].rm_so;

	strncpy(arg, line + matches[1].rm_so, arg_len);
	arg[arg_len] = '\0';
	strncpy(value, line + matches[2].rm_so, value_len);
	value[value_len] = '\0';

	return CONF_OK;
}

conf_error conf_match_comment(const char *line)
{
	regex_t regex;
	regmatch_t matches[1];
	int err;

	err = regcomp(&regex, "^\\s*(#.*)?$", REG_EXTENDED);
	if (err != 0)
		return CONF_MALFORMED_ERROR;

	err = regexec(&regex, line, 1, matches, 0);
	if (err != 0)
		return 0;

	regfree(&regex);
	return 1;
}

conf_error conf_load(const char *conf_path, config *config)
{
	conf_error err;

	FILE *file = fopen(conf_path, "r");
	if (file == NULL)
		return CONF_FOPEN_ERROR;

	size_t memsize = 0, charsize;
	char *line = NULL;
	while ((charsize = getline(&line, &memsize, file)) != -1) {
		if (charsize == 0)
			continue;

		int matches = conf_match_comment(line);
		if (matches < CONF_OK) {
			free(line);
			fclose(file);
			return err;
		}
		if (matches == 1)
			continue;

		char *arg = malloc(charsize);
		if (arg == NULL) {
			free(line);
			fclose(file);
			return CONF_MEMORY_ERROR;
		}
		char *value = malloc(charsize);
		if (value == NULL) {
			free(arg);
			free(line);
			fclose(file);
			return CONF_MEMORY_ERROR;
		}

		err = conf_match_arg(line, arg, value);
		if (err < CONF_OK) {
			free(arg);
			free(value);
			free(line);
			fclose(file);
			return err;
		}

		char *endptr = NULL;
		if (strcmp(arg, "ALLOW") == 0) {
			if (inet_pton(AF_INET, value, &(config->allow)) != 1) {
				fprintf(stderr,
					"Error: Invalid ip address '%s'\n",
					value);

				free(arg);
				free(value);
				fclose(file);
				return CONF_MALFORMED_ERROR;
			}
			break;
		} else if (strcmp(arg, "PORT") == 0) {
			endptr = NULL;
			config->port = strtoul(value, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr,
					"Error: Invalid port number '%s'\n",
					value);

				free(arg);
				free(value);
				fclose(file);
				return CONF_MALFORMED_ERROR;
			}
		} else if (strcmp(arg, "VROOT") == 0) {
			config->vroot = value;
		} else if (strcmp(arg, "MAX_CONNECTION") == 0) {
			endptr = NULL;
			config->max_connections = strtoul(value, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr,
					"Error: Invalid max connections '%s'\n",
					value);

				free(arg);
				free(value);
				fclose(file);
				return CONF_MALFORMED_ERROR;
			}
		} else {
			fprintf(stderr, "Warning: Unknown configuration '%s'\n",
				arg);
		}

		free(arg);
		free(value);
	}
	free(line);

	fclose(file);
	return CONF_OK;
}