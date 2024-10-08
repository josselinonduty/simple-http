#include "magic/magic.h"
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "rfc1945.h"
#include "server.h"

int http_send(const client_t client, const int status_code, char *message)
{
	return http_send_body(client, status_code, message, message);
}

int http_send_body(const client_t client, const int status_code, char *message,
		   char *body)
{
	return http_send_full(client, status_code, message, -1, "text/plain",
			      body);
}

int http_send_full(const client_t client, const int status_code, char *message,
		   long content_length, char *content_type, char *body)
{
	if (content_length < 0)
		content_length = strlen(body);

	long response_length = 0;
	response_length +=
	    strlen(HTTP_VERSION_1_0) + strlen(SP) + 3 + strlen(SP) +
	    strlen(message);
	response_length += strlen(EOL);
	response_length +=
	    strlen("Content-Length:") + strlen(SP) + sizeof(content_length) +
	    strlen(EOL);
	response_length +=
	    strlen("Content-Type:") + strlen(SP) + strlen(content_type) +
	    strlen(EOL);
	response_length += strlen(EOL);
	response_length += content_length;
	response_length += strlen(EOL) + strlen(EOL);

	char *response = malloc(response_length + 1);
	if (NULL == response)
		return -1;

	snprintf(response, response_length,
		 "%s%s%d%s%s%s%s%s%ld%s%s%s%s%s%s%s", HTTP_VERSION_1_0, SP,
		 status_code, SP, message, EOL, "Content-Length:", SP,
		 content_length, EOL, "Content-Type:", SP, content_type,
		 EOL, EOL, body);

	response[response_length] = '\0';

	int err = send(client.socket, response, response_length, 0);
	free(response);

	if (err < 0)
		return err;

	return 0;
}
