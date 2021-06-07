/*
    Copyright 2021 Luke A.C.A. Rieff

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _HTTP_RESPONSE_H
#define _HTTP_RESPONSE_H

#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_header.h"
#include "http_common.h"
#include "http_content_type.h"
#include "http_method.h"
#include "http_version.h"
#include "http_code.h"

#define http_response_set_code(RESPONSE, CODE) ((RESPONSE)->code = (CODE))
#define http_response_set_method(RESPONSE, METHOD) ((RESPONSE)->method = (METHOD))
#define http_response_set_version(RESPONSE, VERSION) ((RESPONSE)->version = (VERSION))

#define http_response_get_version(RESPONSE) ((RESPONSE)->version)
#define http_response_get_code(RESPONSE) ((RESPONSE)->code)

struct http_socket;
typedef struct http_socket http_socket_t;

typedef struct {
    //---//
    http_headers_t *headers;
    //---//
    http_code_t     code;
    http_method_t   method;
    http_version_t  version;
} http_response_t;

/// Creates new HTTP response.
http_response_t *http_response_new (void);

/// Frees an HTTP response.
int32_t http_response_free (http_response_t **response);

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

/// Writes an HTTP response head.
int32_t http_write_response_head (http_socket_t *socket, http_response_t *response);

/// Writes the HTTP response headers.
int32_t http_response_write_headers (http_socket_t *socket, http_response_t *response);

/// Writes an text response to the client.
int32_t http_response_write_text (http_socket_t *socket, http_response_t *response, http_content_type_t type, const char *text);

//./

#endif
