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

#include "http_response.h"
#include "http_socket.h"

const char *X_SERVER_HEADER_KEY = "Server";
const char *X_SERVER_HEADER_VALUE_NAME = "LukeHTTP V1.0";

const char *DATE_HEADER_KEY = "Date";
const char *DATE_HEADER__DATE_FORMAT = "%a, %d %b %Y %X %Z";

const char *CONTENT_TYPE_KEY = "Content-Type";
const char *CONTENT_LENGTH_KEY = "Content-Length";

const char *ACCEPT_RANGES_KEY = "Accept-Ranges";
const char *RANGE_KEY = "Range";
 
http_headers_t *g_DefaultHeaders = NULL;

/// Creates new HTTP response.
http_response_t *http_response_new (void) {
    http_response_t *res = (http_response_t *) calloc (1, sizeof (http_response_t));
    if (res == NULL)
        return NULL;

    res->headers = http_headers_new ();
    if (res->headers == NULL) {
        free (res);
        return NULL;
    }

    return res;
}

/// Frees an HTTP response.
int32_t http_response_free (http_response_t **response) {
    if (http_headers_free (&((*response)->headers)) == -1)
        return -1;
    
    free (*response);
    *response = NULL;

    return 0;
}

/// Adds the X-Server header to the specified headers.
int32_t __http_add_x_server_header (char *buffer, size_t buffer_size, http_headers_t *headers) {
    static const size_t hostname_buffer_size = 128;
    char *hostname_buffer = (char *) malloc (hostname_buffer_size);
    if (hostname_buffer == NULL)
        return -1;

    if (gethostname (hostname_buffer, hostname_buffer_size) != 0) {
        errc_print ("gethostname () failed: ");
        free (hostname_buffer);
        return -1;
    }

    snprintf (buffer, buffer_size, "%s (%s)", hostname_buffer, X_SERVER_HEADER_VALUE_NAME);
    http_headers_insert (headers, X_SERVER_HEADER_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);

    free (hostname_buffer);
    return 0;
}

/// Adds the Date header to the specified headers.
int32_t __http_add_date_header (char *buffer, size_t buffer_size, http_headers_t *headers) {
    time_t t;
    struct tm *t_info;

    t = time (NULL);
    t_info = localtime (&t);

    strftime (buffer, buffer_size, DATE_HEADER__DATE_FORMAT, t_info);
    http_headers_insert (headers, DATE_HEADER_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);

    return 0;
}

int32_t __http_add_content_type_header (char *buffer, size_t buffer_size, http_headers_t *headers, http_content_type_t content_type) {
    const char *content_type_string = http_content_type_to_string (content_type);
    if (content_type_string == NULL)
        return -1;
    
    snprintf (buffer, buffer_size, "%s", content_type_string);
    http_headers_insert (headers, CONTENT_TYPE_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);
    http_headers_insert (headers, "Connection", "keep-alive", HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);

    return 0;
}

int32_t __http_add_content_length_header (char *buffer, size_t buffer_size, http_headers_t *headers, size_t len) {
    snprintf (buffer, buffer_size, "%lu", len);
    http_headers_insert (headers, CONTENT_LENGTH_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);

    return 0;
}

void __http_add_accept_range_header (char *buffer, size_t buffer_size, http_headers_t *headers, http_range_unit_t range) {
    snprintf (buffer, buffer_size, "%s", http_accept_range_to_string (range));
    http_headers_insert (headers, ACCEPT_RANGES_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);
}

/// Gets called at startup of server, prepares the default headers.
int32_t http_response_prepare_default_headers (void) {
    if (g_DefaultHeaders != NULL)
        return -1;

    g_DefaultHeaders = http_headers_new ();
    if (g_DefaultHeaders == NULL)
        return -1;

    static size_t buffer_size = 2048;
    char *buffer = (char *) malloc (buffer_size);
    if (buffer == NULL)
        return -1;

    if (__http_add_x_server_header (buffer, buffer_size, g_DefaultHeaders) == -1) {
        free (buffer);
        return -1;
    }

    free (buffer);
    return 0;
}

/// Gets called to free the default headers.
int32_t http_response_free_default_headers (void) {
    return http_headers_free (&g_DefaultHeaders);
}

/// Adds the default HTTP headers.
int32_t __http_response_add_default_headers (http_response_t *response) {
    static const size_t buffer_size = 2048;
    char *buffer = (char *) malloc (buffer_size);
    if (buffer == NULL)
        return -1;
    
    if (http_headers_add_all (response->headers, g_DefaultHeaders) == -1) {
        free (buffer);
        return -1;
    } else if (__http_add_date_header (buffer, buffer_size, response->headers) ==-1) {
        free (buffer);
        return -1;
    }

    free (buffer);
    
    return 0;
}

/// Writes an text response to the client.
int32_t http_response_write_text (http_socket_t *socket, http_response_t *response, http_content_type_t type, const char *text) {    
    static const size_t buffer_size = 128;
    char *buffer = (char *) malloc (buffer_size);

    if (__http_response_add_default_headers (response) != 0) {
        free (buffer);
        return -1;
    } else if (__http_add_content_type_header (buffer, buffer_size, response->headers, type) != 0) {
        free (buffer);
        return -2;
    } else if (__http_add_content_length_header (buffer, buffer_size, response->headers, strlen (text)) != 0) {
        free (buffer);
        return -3;
    }

    free (buffer);

    http_write_response_head (socket, response);
    http_response_write_headers (socket, response);
    http_socket_enqueue_write_op (socket, http_socket_write_op_create__binary ((uint8_t *) text, strlen (text), true));

    return 0;
}

/// Writes an file to the client.
int32_t http_response_write_file (http_socket_t *socket, http_response_t *response, const char *path) {
    // Opens the file specified in the arguments, if this fails print an error
    //  and return -1.
    FILE *file = fopen (path, "r");
    if (file == NULL) {
        if (errno != ENOENT)
            perror ("fopen () error");
        return -1;
    }

    // Allocates the buffer used for header generation.
    static const size_t buffer_size = 128;
    char *buffer = (char *) malloc (buffer_size);

    // Get the size of the specified file, by seeking to the end, getting the offset
    //  and rewinding.
    fseek (file, 0L, SEEK_END);
    size_t size = ftell (file);
    rewind (file);

    // Gets the content type.
    http_content_type_t type = http_content_type_from_ext (path_get_ext (path));
    if (type == HTTP_CONTENT_TYPE_UNKNOWN)
        type = HTTP_CONTENT_TYPE_APPLICATION_OCTET_STREAM;

    // Adds the default headers, content type and content length headers.
    if (__http_response_add_default_headers (response) != 0) {
        free (buffer);
        return -1;
    } else if (__http_add_content_type_header (buffer, buffer_size, response->headers, type) != 0) {
        free (buffer);
        return -2;
    } else if (__http_add_content_length_header (buffer, buffer_size, response->headers, size) != 0) {
        free (buffer);
        return -3;
    }

    __http_add_accept_range_header (buffer, buffer_size, response->headers, HTTP_ACCEPT_RANGE_BYTES);

    // Sends the HTTP response head, and the headers immediately after.
    http_write_response_head (socket, response);
    http_response_write_headers (socket, response);

    // Checks if we need to write body.
    if (http_response_get_method (response) != HTTP_METHOD_HEAD) {
        http_socket_write_op_t *op = http_socket_write_op_create (HTTP_SOCKET_WRITE_OP_FILE, file, HTTP_SOCKET_WRITE_OP_FLAG__CLOSE_FD);
        if (op == NULL) {
            free (buffer);
            return -1;
        }

        op->size = size;
        http_socket_enqueue_write_op (socket, op);
    } else {
        if (fclose (file) != 0)
            perror ("fclose () failed");
    }

    // Frees thje header buffer, and returns 0.
    free (buffer);
    return 0;
}

void __http_response_write_headers__write_method (const char *header, void *u) {
    http_socket_t *socket = (http_socket_t *) u;
    http_socket_enqueue_write_op (socket, http_socket_write_op_create__binary ((uint8_t *) header, strlen (header), true));
}

/// Writes the HTTP response headers.
int32_t http_response_write_headers (http_socket_t *socket, http_response_t *response) {
    if (http_headers_to_string_no_collapse (response->headers, __http_response_write_headers__write_method, (void *) socket) != 0)
        return -1;

    http_socket_enqueue_write_op (socket, http_socket_write_op_create__binary ((uint8_t *) "\r\n", 2, false));
    return 0;
}

/// Writes an HTTP response head.
int32_t http_write_response_head (http_socket_t *socket, http_response_t *response) {
    char buffer[98];

    // Formats the response string.
    snprintf (buffer, sizeof (buffer) / sizeof (char), "%s %u %s\r\n",
        http_version_to_string (http_response_get_version (response)), http_response_get_code (response),
        http_code_get_message (http_response_get_code (response)));

    // Writes the response string to the socket.
    http_socket_enqueue_write_op (socket, http_socket_write_op_create__binary ((uint8_t *) buffer, strlen (buffer), true));

    return 0;
}
