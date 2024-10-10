#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <ctype.h>

#include "cimap.h"
#include "http.h"
#include "magic/magic.h"
#include "rfc1945.h"
#include "server.h"

int http_request_create(const client_t client, http_request_t *request)
{
	*request = (http_request_t) {
	.major = 0,.minor = 0,.body = (char *)NULL,.body_length = 0,};
	request->headers = cimap_create(16, false);

	char buffer[SERVER_BUFFER_SIZE];
	int total_read = 0;
	int read_size = 0;
	// If the request is sent in multiple packets, read until the end of the headers
	while (total_read < SERVER_BUFFER_SIZE) {
		read_size =
		    recv(client.socket, buffer + total_read,
			 SERVER_BUFFER_SIZE - total_read, 0);
		if (read_size <= 0)
			return read_size;
		total_read += read_size;
		buffer[total_read] = '\0';

		char *end_of_headers = strstr(buffer, EOBLOCK);
		if (end_of_headers)
			break;
	}

	char *end_of_headers = strstr(buffer, EOBLOCK);
	if (!end_of_headers)
		return HTTP_REQUEST_MALFORMED;
	char method[16];

	char parser[32];
	sprintf(parser, "%%s%s%%s%sHTTP/%%d.%%d", SP, SP);
	if (sscanf
	    (buffer, parser, method, request->uri, &request->major,
	     &request->minor) != 4) {
		return HTTP_REQUEST_MALFORMED;
	}

	if (strcmp(method, "GET") == 0) {
		request->method = HTTP_METHOD_GET;
	} else if (strcmp(method, "HEAD") == 0) {
		request->method = HTTP_METHOD_HEAD;
	} else if (strcmp(method, "POST") == 0) {
		request->method = HTTP_METHOD_POST;
	} else {
		return HTTP_REQUEST_MALFORMED;
	}

	char *line_start = strchr(buffer, '\n') + 1;
	while (line_start < end_of_headers) {
		char *line_end = strchr(line_start, '\n');
		if (!line_end)
			break;

		*line_end = '\0';
		char *colon = strchr(line_start, ':');
		if (colon) {
			*colon = '\0';
			char *key = line_start;
			char *value = colon + 1;
			while (*value == ' ')
				value++;
			cimap_set(request->headers, key, value);
		}
		line_start = line_end + 1;
	}

	// Handle body if present
	const char *content_length_str =
	    cimap_get(request->headers, "Content-Length");
	if (content_length_str) {
		size_t content_length = atoi(content_length_str);
		if (content_length > 0) {
			request->body_length = content_length;
			if (content_length > SERVER_BODY_SIZE)
				return HTTP_ENTITY_TOO_LARGE;
			request->body = malloc(content_length + 1);
			if (!request->body)
				return -1;

			size_t body_received =
			    total_read - (end_of_headers - buffer + 4);
			memcpy(request->body, end_of_headers + 4,
			       body_received);

			while (body_received < content_length) {
				read_size =
				    recv(client.socket,
					 request->body + body_received,
					 content_length - body_received, 0);
				if (read_size <= 0) {
					free(request->body);
					request->body = NULL;
					return -1;
				}
				body_received += read_size;
			}
			request->body[content_length] = '\0';
		}
	}

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client.client_addr.sin_addr, ip, INET_ADDRSTRLEN);
	fprintf(stderr, "[%s] %s %s HTTP/%d.%d\n", ip, method,
		request->uri, request->major, request->minor);

	return 0;
}

void http_request_destroy(http_request_t *request)
{
	cimap_free(request->headers);

	if (NULL != request->body) {
		free(request->body);
	}
}

int http_response_create(http_response_t *response)
{
	*response = (http_response_t) {
	.status_code = 200,.major = 0,.minor = 0,.body =
		    (char *)NULL,.body_length = 0};
	response->headers = cimap_create(16, false);

	http_response_status(response, 200);
	cimap_set(response->headers, "Server", "josselinonduty/simple-http");

	return 0;
}

char *http_response_message(int status_code)
{
	switch (status_code) {
	case 200:
		return STATUS_TEXT_200;
	case 201:
		return STATUS_TEXT_201;
	case 202:
		return STATUS_TEXT_202;
	case 204:
		return STATUS_TEXT_204;

	case 300:
		return STATUS_TEXT_300;
	case 301:
		return STATUS_TEXT_301;
	case 302:
		return STATUS_TEXT_302;
	case 304:
		return STATUS_TEXT_304;

	case 400:
		return STATUS_TEXT_400;
	case 401:
		return STATUS_TEXT_401;
	case 403:
		return STATUS_TEXT_403;
	case 404:
		return STATUS_TEXT_404;

	case 500:
		return STATUS_TEXT_500;
	case 501:
		return STATUS_TEXT_501;
	case 502:
		return STATUS_TEXT_502;
	case 503:
		return STATUS_TEXT_503;

	default:
		return NULL;
	}
}

int http_response_body(http_response_t *response, const char *body)
{
	// Copy the body
	response->body_length = strlen(body);
	response->body = malloc(response->body_length + 1);
	if (NULL == response->body)
		return -1;

	strcpy(response->body, body);
	response->body[response->body_length] = '\0';
	return 0;
}

int http_response_status(http_response_t *response, int status_code)
{
	response->status_code = status_code;
	return 0;
}

int http_response_send(const client_t client, const http_request_t *request,
		       http_response_t *response)
{
	char buffer[SERVER_BUFFER_SIZE];

	// Send status line: "HTTP/1.0 200 OK\r\n"
	int write_size = snprintf(buffer, SERVER_BUFFER_SIZE, "%s%s%d%s%s%s",
				  HTTP_VERSION_1_0, SP, response->status_code,
				  SP,
				  http_response_message(response->status_code),
				  EOL);
	int err = send(client.socket, buffer, write_size, 0);

	// Send headers
	cimap_iterator_t *iterator = cimap_iterator(response->headers);
	const char *key, *value;
	while (cimap_next(iterator, &key, &value) == 0) {
		write_size =
		    snprintf(buffer, SERVER_BUFFER_SIZE, "%s:%s%s%s", key, SP,
			     value, EOL);
		err = send(client.socket, buffer, write_size, 0);
		if (err < 0)
			return err;
	}
	cimap_iterator_free(iterator);

	err = send(client.socket, EOL, 2, 0);
	if (err < 0)
		return err;

	if (request->method == HTTP_METHOD_HEAD) {
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client.client_addr.sin_addr, ip,
			  INET_ADDRSTRLEN);
		fprintf(stderr, "[%s] %d %s\n", ip, response->status_code,
			http_response_message(response->status_code));

		return 0;
	}
	// Send body by blocks of SERVER_BUFFER_SIZE bytes
	if (NULL != response->body) {
		size_t body_length = response->body_length;
		size_t sent = 0;
		while (sent < body_length) {
			size_t to_send =
			    body_length - sent >
			    SERVER_BUFFER_SIZE ? SERVER_BUFFER_SIZE :
			    body_length - sent;
			err =
			    send(client.socket, response->body + sent, to_send,
				 0);
			if (err < 0)
				return err;
			sent += to_send;
		}
		if (err < 0)
			return err;
	}

	err = send(client.socket, EOL, 2, 0);
	if (err < 0)
		return err;

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client.client_addr.sin_addr, ip, INET_ADDRSTRLEN);
	fprintf(stderr, "[%s] %d %s\n", ip, response->status_code,
		http_response_message(response->status_code));

	return 0;
}

int http_response_send_file(const client_t client,
			    const http_request_t *request,
			    http_response_t *response, const char *file_name)
{
	FILE *fd = fopen(file_name, "r");
	if (NULL == fd) {
		if (ENOENT == errno) {
			http_response_status(response, 404);
			http_response_body(response, STATUS_TEXT_404);
		} else {
			http_response_status(response, 500);
		}
		return http_response_send(client, request, response);
	}
	char buffer[SERVER_BUFFER_SIZE];

	// Send status line: "HTTP/1.0 200 OK\r\n"
	int write_size = snprintf(buffer, SERVER_BUFFER_SIZE, "%s%s%d%s%s%s",
				  HTTP_VERSION_1_0, SP, response->status_code,
				  SP,
				  http_response_message(response->status_code),
				  EOL);
	int err = send(client.socket, buffer, write_size, 0);

	// Determine the file size
	fseek(fd, 0L, SEEK_END);
	size_t file_size = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	int content_length_len = snprintf(NULL, 0, "%ld", file_size);
	char *content_length = malloc(content_length_len + 1);
	snprintf(content_length, content_length_len + 1, "%ld", file_size);
	content_length[content_length_len] = '\0';
	cimap_set(response->headers, "Content-Length", content_length);
	free(content_length);

	// Send headers
	cimap_iterator_t *iterator = cimap_iterator(response->headers);
	const char *key, *value;
	while (cimap_next(iterator, &key, &value) == 0) {
		write_size =
		    snprintf(buffer, SERVER_BUFFER_SIZE, "%s:%s%s%s", key, SP,
			     value, EOL);
		err = send(client.socket, buffer, write_size, 0);
		if (err < 0)
			return err;
	}
	cimap_iterator_free(iterator);

	err = send(client.socket, EOL, 2, 0);
	if (err < 0)
		return err;

	if (request->method == HTTP_METHOD_HEAD) {
		fclose(fd);

		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client.client_addr.sin_addr, ip,
			  INET_ADDRSTRLEN);
		fprintf(stderr, "[%s] %d %s\n", ip, response->status_code,
			http_response_message(response->status_code));

		return 0;
	}
	// Send body (file)
	size_t read_size;
	while ((read_size = fread(buffer, 1, SERVER_BUFFER_SIZE, fd)) > 0) {
		err = send(client.socket, buffer, read_size, 0);
		if (err < 0)
			return err;

		if (read_size < SERVER_BUFFER_SIZE)
			break;
	}

	fclose(fd);

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client.client_addr.sin_addr, ip, INET_ADDRSTRLEN);
	fprintf(stderr, "[%s] %d %s\n", ip, response->status_code,
		http_response_message(response->status_code));

	return 0;
}

void http_response_destroy(http_response_t *response)
{
	cimap_free(response->headers);

	if (NULL != response->body) {
		free(response->body);
	}
}

static magic_t magic_cookie = NULL;

int http_content_init(void)
{
	magic_cookie = magic_open(MAGIC_MIME | MAGIC_ERROR);
	if (NULL == magic_cookie)
		return -1;

	if (0 != magic_load(magic_cookie, NULL))
		return -1;

	fprintf(stderr, "Info: Magic database loaded\n");
	return 0;
}

int http_content_free(void)
{
	magic_close(magic_cookie);
	return 0;
}

const char *http_content_get_override(const char *file_name)
{
	const char *extension = strrchr(file_name, '.');
	if (NULL == extension)
		return NULL;

	if (strcmp(extension, ".css") == 0)
		return "text/css";
	if (strcmp(extension, ".js") == 0)
		return "application/javascript";
	if (strcmp(extension, ".json") == 0)
		return "application/json";
	if (strcmp(extension, ".xml") == 0)
		return "application/xml";
	if (strcmp(extension, ".md") == 0)
		return "text/markdown";

	return NULL;
}

int http_content_get(const char *file_name, char **content_type)
{
	if (NULL == magic_cookie) {
		fprintf(stderr, "Error: %s", magic_error(magic_cookie));
		return -1;
	}

	const char *mime_type = http_content_get_override(file_name);
	if (NULL == mime_type) {
		mime_type = magic_file(magic_cookie, file_name);
		if (NULL == mime_type) {
			return HTTP_ENTITY_NOT_FOUND;
		}
	}

	strcpy(*content_type, mime_type);
	(*content_type)[strlen(mime_type)] = '\0';

	return 0;
}
