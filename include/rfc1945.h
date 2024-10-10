#ifndef RFC1945_H
#define RFC1945_H

// RFC 1945: Hypertext Transfer Protocol -- HTTP/1.0

/* ---------- HTTP Version ---------- */
#define HTTP_VERSION_0_9 "HTTP/0.9"
#define HTTP_VERSION_1_0 "HTTP/1.0"
/* ---------------------------------- */

/* ---------- Special Characters ---------- */
#define CR "\x0D"
#define LF "\x0A"
#define SP "\x20"
#define HT "\x09"
#define DQUOTE "\x22"
#define CRLF "\x0D\x0A"
#define EOL CRLF
#define EOBLOCK "\x0D\x0A\x0D\x0A"
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

/* ---------- HTTP Method ---------- */
#define METHOD_GET "GET"
#define METHOD_HEAD "HEAD"
#define METHOD_POST "POST"
/* --------------------------------- */

/* ---------- HTTP Character Set ---------- */
#define CHARSET_US_ASCII "US-ASCII"
#define CHARSET_ISO_8859_1 "ISO-8859-1"
#define CHARSET_ISO_8859_2 "ISO-8859-2"
#define CHARSET_ISO_8859_3 "ISO-8859-3"
#define CHARSET_ISO_8859_4 "ISO-8859-4"
#define CHARSET_ISO_8859_5 "ISO-8859-5"
#define CHARSET_ISO_8859_6 "ISO-8859-6"
#define CHARSET_ISO_8859_7 "ISO-8859-7"
#define CHARSET_ISO_8859_8 "ISO-8859-8"
#define CHARSET_ISO_8859_9 "ISO-8859-9"
#define CHARSET_ISO_2022_JP "ISO-2022-JP"
#define CHARSET_ISO_2022_JP_2 "ISO-2022-JP-2"
#define CHARSET_ISO_2022_KR "ISO-2022-KR"
#define CHARSET_UNICODE_1_1 "UNICODE-1-1"
#define CHARSET_UNICODE_1_1_UTF_7 "UNICODE-1-1-UTF-7"
#define CHARSET_UNICODE_1_1_UTF_8 "UNICODE-1-1-UTF-8"
/* --------------------------------------- */

/* ---------- HTTP Content Coding ---------- */
#define CONTENT_CODING_GZIP "gzip"
#define CONTENT_CODING_X_GZIP "x-gzip"
#define CONTENT_CODING_COMPRESS "compress"
#define CONTENT_CODING_X_COMPRESS "x-compress"
/* ----------------------------------------- */

/* ---------- Miscellaneous ---------- */
#define SERVER_NAME "josselinonduty/simple-http"
#define SERVER_BUFFER_SIZE 8192
#define SERVER_BODY_SIZE 4294967295
/* ----------------------------------- */

#endif