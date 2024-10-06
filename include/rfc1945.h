#ifndef RFC1945_H
#define RFC1945_H

// RFC 1945: Hypertext Transfer Protocol -- HTTP/1.0

/* ---------- HTTP Version ---------- */
#define HTTP_VERSION "HTTP/1.0"
#define HTTP_VERSION_MAJOR 1
#define HTTP_VERSION_MINOR 0
/* ---------------------------------- */

/* ---------- Special Characters ---------- */
#define CR "\x0D"
#define LF "\x0A"
#define SP "\x20"
#define HT "\x09"
#define DQUOTE "\x22"
#define CRLF "\x0D\x0A"
#define EOL CRLF
/* ---------------------------------------- */

/* ---------- STATUS_TEXT Code ---------- */
// 1xx: Informational

// 2xx: Success
#define STATUS_TEXT_200 "OK"
#define STATUS_TEXT_201 "Created"
#define STATUS_TEXT_202 "Accepted"
#define STATUS_TEXT_204 "No Content"

// 3xx: Redirection
#define STATUS_TEXT_300 "Multiple Choices"
#define STATUS_TEXT_301 "Moved Permanently"
#define STATUS_TEXT_302 "Moved Temporarily"
#define STATUS_TEXT_304 "Not Modified"

// 4xx: Client Error
#define STATUS_TEXT_400 "Bad Request"
#define STATUS_TEXT_401 "Unauthorized"
#define STATUS_TEXT_403 "Forbidden"
#define STATUS_TEXT_404 "Not Found"
#define STATUS_TEXT_418 "I'm a teapot"

// 5xx: Server Error
#define STATUS_TEXT_500 "Internal Server Error"
#define STATUS_TEXT_501 "Not Implemented"
#define STATUS_TEXT_502 "Bad Gateway"
#define STATUS_TEXT_503 "Service Unavailable"
/* ------------------------------------- */

#endif