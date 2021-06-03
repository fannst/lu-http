#include "http_response.h"

const char *X_SERVER_HEADER_KEY = "X-Server";
const char *X_SERVER_HEADER_VALUE_NAME = "LukeHTTP V1.0";

const char *DATE_HEADER_KEY = "Date";
const char *DATE_HEADER__DATE_FORMAT = "%a, %d %b %Y %X %Z";
 
http_headers_t *g_DefaultHeaders = NULL;

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
    http_headers_insert (headers, DATE_HEADER_KEY, buffer, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_END);

    return 0;
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
