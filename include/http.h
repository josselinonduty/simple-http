#ifndef HTTP_H
#define HTTP_H

#include "server.h"

typedef enum http_method_t {
    HTTP_METHOD_GET = 1,
    HTTP_METHOD_HEAD = 2,
    HTTP_METHOD_POST = 3,
} http_method_t;

// TODO: Implement http_request_t
typedef void *http_request_t;

typedef enum http_error {
    HTTP_OK = 0,
    HTTP_REQUEST_MALFORMED = -10,
} http_error;

http_error http_request_parse(const char *request, http_method_t *method, char *uri, int *major, int *minor);

int http_send(const client_t client, const int status_code, char *message);
int http_send_body(const client_t client, const int status_code, char *message, char *body);
int http_send_full(const client_t client, const int status_code, char *message, long content_length, char *content_type, char *body);

int http_get_content_type_encoding(FILE *fd, char *content_type, char *content_encoding);
#endif