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
