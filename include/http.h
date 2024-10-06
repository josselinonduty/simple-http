#ifndef HTTP_H
#define HTTP_H

#include "server.h"

int http_send(const client_t client, const int status_code, char *message);
int http_send_body(const client_t client, const int status_code, char *message, char *body);
int http_send_full(const client_t client, const int status_code, char *message, long content_length, char *content_type, char *body);

#endif