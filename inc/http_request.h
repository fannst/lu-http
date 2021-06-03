#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "http_socket.h"

typedef enum {
    HTTP_METHOD_HEAD, HTTP_METHOD_GET, HTTP_METHOD_POST,
    HTTP_METHOD_PUT, HTTP_METHOD_PATCH
} http_method_t;

typedef struct {
    /* Connection Info */
    http_socket_t  *socket;
    /* Request Info */
    http_method_t   method;
} http_request_t;

#endif
