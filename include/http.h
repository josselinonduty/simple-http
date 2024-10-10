#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include <stdio.h>

#include "cimap.h"
#include "rfc1945.h"
#include "server.h"

typedef enum http_method_t {
    HTTP_METHOD_GET = 1,
    HTTP_METHOD_HEAD = 2,
    HTTP_METHOD_POST = 3,
} http_method_t;

typedef struct http_request_t {
    http_method_t method;
    char uri[SERVER_BUFFER_SIZE];
    int major;
    int minor;
    cimap_t *headers;
    char *body;
    size_t body_length;
} http_request_t;

typedef struct http_response_t {
    int status_code;
    int major;
    int minor;
    cimap_t *headers;
    char *body;
    size_t body_length;
} http_response_t;

typedef enum http_error {
    HTTP_OK = 0,
    HTTP_REQUEST_MALFORMED = -10,
    HTTP_ENTITY_NOT_FOUND = -20,
    HTTP_ENTITY_TOO_LARGE = -21,
} http_error;

int http_request_create(const client_t client, http_request_t *request);
void http_request_destroy(http_request_t *request);

int http_response_create(http_response_t *response);
int http_response_status(http_response_t *response, int status_code);
int http_response_body(http_response_t *response, const char *body);
int http_response_send(const client_t client, const http_request_t *request, http_response_t *response);
int http_response_send_file(const client_t client, const http_request_t *request, http_response_t *response, const char *file_name);
void http_response_destroy(http_response_t *response);

int http_content_init(void);
int http_content_get(const char *file_name, char **content_type);
int http_content_free(void);

int http_send(const client_t client, const http_response_t *response);
int http_send_body(const client_t client, const http_response_t *response);
int http_send_full(const client_t client, const http_response_t *response);

#endif