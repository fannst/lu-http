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

#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "http_socket.h"
#include "http_header.h"
#include "http_line_buffer.h"

typedef enum {
    HTTP_REQUEST_STATE_RECEIVING_TYPE = 0,
    HTTP_REQUEST_STATE_RECEIVING_HEADERS,
    HTTP_REQUEST_STATE_RECEIVING_BODY
} http_request_state_t;

typedef enum {
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2,
    HTTP_VERSION_3,
    HTTP_VERSION_INVALID
} http_version_t;

typedef enum {
    HTTP_METHOD_GET, HTTP_METHOD_HEAD, HTTP_METHOD_POST, HTTP_METHOD_PUT, HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE, HTTP_METHOD_OPTIONS, HTTP_METHOD_CONNECT, HTTP_METHOD_PATCH,
    HTTP_METHOD_INVALID
} http_method_t;

typedef struct {
    http_request_state_t    state;

    http_method_t           method;
    http_version_t          version;

    http_headers_t         *headers;

    char                   *url;

    __http_line_buffer_t   *body;
} http_request_t;

/// Prints HTTP request info.
void http_request_print (http_request_t *request);

/// Parses an HTTP method from string.
http_method_t http_method_from_string (const char *str);

/// Parses an HTTP version from string.
http_version_t http_version_from_string (const char *str);

/// Returns the string version of HTTP method.
const char *http_method_to_string (http_method_t method);

/// Returns the string version of version.
const char *http_version_to_string (http_version_t version);

/// Updates the HTTP request when type state is specified.
int32_t __http_request_update__type (http_request_t *request, char *line);

/// Updates the HTTP request when header state is specified.
int32_t __http_request_update__headers (http_request_t *request, char *line);

/// Updates the HTTP request with the specified line, this will process it further.
int32_t http_request_update (http_request_t *request, char *line);

#endif
