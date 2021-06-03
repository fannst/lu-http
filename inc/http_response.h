#ifndef _HTTP_RESPONSE_H
#define _HTTP_RESPONSE_H

#include <stdint.h>
#include <time.h>

#include "http_header.h"
#include "http_common.h"
#include "http_socket.h"
#include "http_content_type.h"

#define http_response_set_code (RESPONSE, CODE) ((RESPONSE)->code = (CODE))

typedef struct {
    /* Connection Info */
    http_socket_t  *socket;
    /* Response Info */
    http_headers_t *headers;
    uint32_t        code;
} http_response_t;

/// Adds the X-Server header to the specified headers.
int32_t __http_add_x_server_header (char *buffer, size_t buffer_size, http_headers_t *headers);

/// Adds the Date header to the specified headers.
int32_t __http_add_date_header (char *buffer, size_t buffer_size, http_headers_t *headers);

/// Gets called at startup of server, prepares the default headers.
int32_t http_response_prepare_default_headers (void);

/// Gets called to free the default headers.
int32_t http_response_free_default_headers (void);

/// Adds the default HTTP headers.
int32_t __http_response_add_default_headers (http_response_t *response);

/// Writes an text response to the client.
int32_t http_response_write_text (http_response_t *response, const char *text);

#endif
