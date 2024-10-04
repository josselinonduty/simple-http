#ifndef CONF_H
#define CONF_H

typedef struct config
{
    int host;
    int port;
    char *vroot;
    int max_connections;
} config;

typedef enum conf_error
{
    CONF_OK = 0,
    CONF_FOPEN_ERROR = -1,
    CONF_MALFORMED_ERROR = -2,
    CONF_MEMORY_ERROR = -3
} conf_error;

conf_error conf_load(const char *conf_path, config *config);

#endif