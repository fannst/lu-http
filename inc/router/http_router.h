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

#ifndef _ROUTER_HTTP_ROUTER_H
#define _ROUTER_HTTP_ROUTER_H

#include "../http_request.h"
#include "../http_response.h"
#include "../http_socket.h"

#define http_route_flag_set(ROUTE, FLAG) \
  ((ROUTE)->flags) |= FLAG
#define http_route_flag_clear(ROUTE, FLAG) \
  ((ROUTE)->flags) &= ~FLAG
#define http_route_flag_is_set(ROUTE, FLAG) \
  ((((ROUTE)->flags) & (FLAG)) != 0)

typedef enum {
  HTTP_ROUTE_TYPE__CALLBACK = 0,
  HTTP_ROUTE_TYPE__SUBROUTER
} http_route_type_t;

typedef enum {
  HTTP_ROUTE_FLAG__MATCH_ALL = 0,
} http_route_flag_t;

struct http_route {
  http_route_type_t type;
  void *data;
  void *u;
  const char *path;
  struct http_route *next;
  uint32_t flags;
};

typedef struct http_route http_route_t;

typedef struct {
  http_route_t *entry;
} http_router_t;

typedef void (*http_route_callback)(http_socket_t *, const http_request_t *,
                                    http_response_t *, const char *, void *u);

/// Registers an callback route.
int32_t http_router__register_callback(http_router_t *router, const char *path,
                                       http_route_callback callback, void *u);

/// Registers an subroute route.
http_router_t *http_router__register_subroute(http_router_t *router,
                                              const char *path);

/// Uses an HTTP router.
int32_t http_router_use(http_router_t *router, http_socket_t *socket,
                        const http_request_t *request,
                        http_response_t *response, const char *path);

#endif
