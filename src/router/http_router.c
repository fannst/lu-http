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

#include "router/http_router.h"

/// Registers an callback route.
int32_t http_router__register_callback(http_router_t *router, const char *path,
                                       http_route_callback callback, void *u) {
  // Allocates the memory required for the route.
  http_route_t *route = (http_route_t *)calloc(1, sizeof(http_route_t));
  if (route == NULL)
    return -1;

  // Sets the values inside the route.
  route->path = path;
  route->type = HTTP_ROUTE_TYPE__CALLBACK;
  route->data = (void *)callback;
  route->u = u;

  // Sets the current router entry as next to the route to be added
  //  and sets the new router entry to the currently added route.
  route->next = router->entry;
  router->entry = route;

  return 0;
}

/// Registers an subroute route.
http_router_t *http_router__register_subroute(http_router_t *router,
                                              const char *path) {
  // Allocates the memory required for the sub router.
  http_router_t *sub_router = (http_router_t *)calloc(1, sizeof(http_router_t));
  if (sub_router == NULL)
    return NULL;

  // Allocates the memory required for the route.
  http_route_t *route = (http_route_t *)calloc(1, sizeof(http_route_t));
  if (route == NULL) {
    free(sub_router);
    return NULL;
  }

  // Sets the route values.
  route->path = path;
  route->type = HTTP_ROUTE_TYPE__SUBROUTER;
  route->data = (void *)sub_router;

  // Sets the current router entry as next to the route to be added
  //  and sets the new router entry to the currently added route.
  route->next = router->entry;
  router->entry = route;

  return sub_router;
}

/// Uses an HTTP router.
int32_t http_router_use(http_router_t *router, http_socket_t *socket,
                        const http_request_t *request,
                        http_response_t *response, const char *path) {
  // Allocates the memory for the strtok_r function, if this fails return error
  //  else copy the path into the strtok path.
  char *strtok_mem = malloc(strlen(path) + 1);
  if (strtok_mem == NULL) {
    return -1;
  }

  memcpy(strtok_mem, path, strlen(path) + 1);

  // Loops over all the parts of the url, and starts matching the routes we
  // have.
  char *save_ptr;
  char *token = strtok_r(strtok_mem, "/", &save_ptr);

  while (token != NULL) {
    // Checks for any matching routes.
    http_route_t *route = router->entry;
    while (route != NULL) {
      // Checks if we're matching, if not continue to next round.
      if (strcmp(token, route->path) != 0) {
        route = route->next;
        continue;
      }

      // Checks the type of match, if callback just call the function
      //  else search the sub-router.
      if (route->type == HTTP_ROUTE_TYPE__CALLBACK) {
        token = strtok_r(NULL, "/", &save_ptr);

        // Checks if there is a remaining path, if so, if the route is match all
        //  otherwise ignore the route.
        if (!http_route_flag_is_set(route, HTTP_ROUTE_FLAG__MATCH_ALL) && token != NULL) {
          route = route->next;
          continue;
        }

        // Gets the remaining path, if any at all.
        const char *remaining_path = NULL;
        if (token != NULL) {
          size_t i = token - strtok_mem;
          remaining_path = &path[i];
        }

        // Calls the callback.
        ((http_route_callback)(route->data))(socket, request, response,
                                             remaining_path, route->u);
      } else if (route->type == HTTP_ROUTE_TYPE__SUBROUTER) {
        router = (http_router_t *)route->data;
        break;
      }

      free(strtok_mem);
      return 0;
    }

    token = strtok_r(NULL, "/", &save_ptr);
  }

  // Frees the memory, and returns 1 since we've not found any matching route
  //  and thus need to render 404.
  free(strtok_mem);
  return 1;
}
