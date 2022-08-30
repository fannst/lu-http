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

#include "main.h"
#include "http_content_type.h"
#include "http_request.h"
#include "http_response.h"
#include "router/http_router.h"

http_router_t router = {.entry = NULL};

void print_header(const char *memory, void *u) { printf("%s", memory); }

void *recurring_thread(void *u) {
  for (;;) {
    malloc_trim(1024);
    usleep(500 * 1000);
  }
}

void on_http_request(http_socket_t *socket, const http_request_t *request,
                     http_response_t *response) {
  printf("%s\n", request->parsed_url.path);
  int32_t rc = http_router_use(&router, socket, request, response,
                               request->parsed_url.path);

  if (rc == 1) {
    http_response_set_code(response, 404);
    http_response_write_file(socket, response, "./html/404.html");
  }
}

void static_route(http_socket_t *socket, const http_request_t *request,
                  http_response_t *response, const char *path, void *u) {
  char *file_path = malloc(strlen(path) + strlen((const char *)u) + 2);
  sprintf(file_path, "%s/%s", (const char *)u, path);

  http_response_set_code(response, 200);
  int32_t rc = http_response_write_file(socket, response, file_path);
  if (rc == -1) {
    http_response_set_code(response, 404);
    http_response_write_file(socket, response, "./html/404.html");
  }
}

void test_route(http_socket_t *socket, const http_request_t *request,
                http_response_t *response, const char *path, void *u) {
  http_response_set_code(response, 200);
  http_response_write_text(socket, response, HTTP_CONTENT_TYPE_TEXT_PLAIN,
                           "test!");
}

void __main_register_routes() {
  http_router__register_callback(&router, "static", static_route, "./static");
  http_router__register_callback(&router, "test", test_route, NULL);
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  // Handles the arguments.

  // Prints some deserved credits.
  printf(
      "Lu-HTTP is created by Luke A.C.A. Rieff, it's super fast. I know.\r\n");

  // Creates the thread which will perform some small tasks, for example
  //  trimming the allocated memory.
  pthread_t thread;
  pthread_create(&thread, NULL, recurring_thread, NULL);

  // Registers the route.
  __main_register_routes();

  http_response_prepare_default_headers();
  http_helpers_init();

  http_server_socket_t *sock =
      http_server_socket_create(10, 1024, on_http_request);

  http_server_socket_init(sock);
  http_server_socket_configure(sock, 8080, "0.0.0.0", 20);
  http_server_socket_bind(sock);
  http_server_socket_listen(sock);
  http_server_start_acceptor(sock, true);
  http_server_start_thread_pools(sock);

  printf("Press ENTER to shut down\r\n");
  getchar();

  pthread_cancel(thread);
  pthread_join(thread, NULL);

  http_server_socket_stop(sock);
  http_server_socket_free(&sock);

  http_response_free_default_headers();
  return 0;
}
