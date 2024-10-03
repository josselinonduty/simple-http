#ifndef CLI_H
#define CLI_H

typedef enum cli_error {
    cli_ok = 0,
    cli_args_memory = -1,
    cli_args_duplicate = -2,
    cli_conversion_error = -3,
    cli_config_file_error = -4,
    cli_config_file_malformed = -5,
    cli_config_error = -6,
    cli_usage_error = -7
} cli_error;

typedef struct config {
    int allow;
    int port;
    char* vroot;
    int max_connections;
} config;

cli_error cli_load(int argc, char **argv, config* config);

#endif