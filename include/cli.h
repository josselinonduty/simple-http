#ifndef CLI_H
#define CLI_H

typedef enum cli_error {
    cli_ok = 0,
    cli_args_memory,
    cli_args_duplicate,
    cli_conversion_error,
    cli_config_file_error,
    cli_config_file_malformed,
    cli_config_error,
    cli_usage_error,
} cli_error;

typedef struct config {
    int allow;
    int port;
    char* vroot;
} config;

cli_error cli_load(int argc, char **argv, config* config);

#endif