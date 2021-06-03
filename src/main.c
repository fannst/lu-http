#include "main.h"

void print_header (const char *memory, void *u) {
    printf ("%s", memory);
}

int main (int argc, char **argv) {
    srand (time (NULL));

//     http_response_prepare_default_headers ();
    http_helpers_init ();

    http_server_socket_t *sock = NULL;
    http_server_socket_new (&sock, 1);
    http_server_socket_configure (sock, 8080, "0.0.0.0", 20);
    http_server_socket_bind (sock);
    http_server_socket_listen (sock);
    http_server_start_acceptor (sock, true);
    http_server_start_thread_pools (sock);

    printf ("Press ENTER to shut down\r\n");
    getchar();

    http_server_socket_free (&sock);
//
//     http_response_t resp = {
//         .headers = http_headers_new ()
//     };
//
//     __http_response_add_default_headers (&resp);
//
//     http_headers_to_string_no_collapse (resp.headers, print_header, NULL);
//
//     http_headers_free (&resp.headers);
//
//     const char *headers = "Some-Header: Hello World!\r\n"
//     "Some-Other-Header: Cool!\r\n"
//     "Some-Other-Header Cool!\r\n"
//     "\r\n";
//
//     http_headers_t *h = parse_http_headers ((char *) headers, HTTP_PARSE_HEADER_FLAG_KEEP_INTACT);
//     http_headers_free (&h);

//     http_response_free_default_headers ();
    return 0;
}
