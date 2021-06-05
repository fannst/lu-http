#include "http_request.h"

/// Parses an HTTP method from string.
http_method_t http_method_from_string (const char *str) {
    if (strcmp (str, "GET") == 0)
        return HTTP_METHOD_GET;
    else if (strcmp (str, "HEAD") == 0)
        return HTTP_METHOD_HEAD;
    else if (strcmp (str, "POST") == 0)
        return HTTP_METHOD_POST;
    else if (strcmp (str, "PUT") == 0)
        return HTTP_METHOD_PUT;
    else if (strcmp (str, "DELETE") == 0)
        return HTTP_METHOD_DELETE;
    else if (strcmp (str, "TRACE") == 0)
        return HTTP_METHOD_TRACE;
    else if (strcmp (str, "OPTIONS") == 0)
        return HTTP_METHOD_OPTIONS;
    else if (strcmp (str, "CONNECT") == 0)
        return HTTP_METHOD_CONNECT;
    else if (strcmp (str, "PATCH") == 0)
        return HTTP_METHOD_PATCH;

    return HTTP_METHOD_INVALID;
}

/// Returns the string version of HTTP method.
const char *http_method_to_string (http_method_t method) {
    switch (method) {
        case HTTP_METHOD_GET:
            return "GET";
        case HTTP_METHOD_HEAD:
            return "HEAD";
        case HTTP_METHOD_POST:
            return "POST";
        case HTTP_METHOD_PUT:
            return "PUT";
        case HTTP_METHOD_DELETE:
            return "DELETE";
        case HTTP_METHOD_TRACE:
            return "TRACE";
        case HTTP_METHOD_OPTIONS:
            return "OPTIONS";
        case HTTP_METHOD_CONNECT:
            return "CONNECT";
        case HTTP_METHOD_PATCH:
            return "PATCH";
        case HTTP_METHOD_INVALID:
        default:
            return NULL;
    }
}

/// Parses an HTTP version from string.
http_version_t http_version_from_string (const char *str) {
    if (strcmp (str, "HTTP/1.0") == 0)
        return HTTP_VERSION_1_0;
    else if (strcmp (str, "HTTP/1.1") == 0)
        return HTTP_VERSION_1_1;
    else if (strcmp (str, "HTTP/2") == 0)
        return HTTP_VERSION_2;
    else if (strcmp (str, "HTTP/3") == 0)
        return HTTP_VERSION_3;
    
    return HTTP_VERSION_INVALID;
}

/// Returns the string version of version.
const char *http_version_to_string (http_version_t version) {
    switch (version) {
        case HTTP_VERSION_1_0:
            return "HTTP/1.0";
        case HTTP_VERSION_1_1:
            return "HTTP/1.1";
        case HTTP_VERSION_2:
            return "HTTP/2";
        case HTTP_VERSION_3:
            return "HTTP/3";
        case HTTP_VERSION_INVALID:
        default:
            return NULL;
    }
}

/// Prints HTTP request info.
void __http_request_print__header_method (const char *str, void *u) {
    printf ("\t%s", str);
}

void http_request_print (http_request_t *request) {
    printf ("HTTP request:\r\n");
    printf ("- Method: %s\r\n", http_method_to_string (request->method));
    printf ("- URL: %s\r\n", request->url);
    printf ("- Version: %s\r\n", http_version_to_string (request->version));
    printf ("- Headers:\r\n");
    http_headers_to_string_no_collapse (request->headers, __http_request_print__header_method, NULL);
}

/// Updates the HTTP request when type state is specified.
int32_t __http_request_update__type (http_request_t *request, char *line) {
    char *save_ptr = NULL, *tok = NULL;
    size_t len;

    // HTTP Method

    if ((tok = strtok_r (line, " ", &save_ptr)) == NULL)
        return -1;
    
    request->method = http_method_from_string (tok);

    // Path

    if ((tok = strtok_r (NULL, " ", &save_ptr)) == NULL)
        return -1;

    len = strlen (tok) + 1;
    request->url = (char *) malloc (len);
    memcpy (request->url, tok, len);
    
    // Version

    if ((tok = strtok_r (NULL, " ", &save_ptr)) == NULL)
        return -1;
    
    request->version = http_version_from_string (tok);

    // Sets the next state
    request->state = HTTP_REQUEST_STATE_RECEIVING_HEADERS;

    return 0;
}

/// Updates the HTTP request when header state is specified.
int32_t __http_request_update__headers (http_request_t *request, char *line) {
    if (strcmp (line, "") == 0) {
        request->state = HTTP_REQUEST_STATE_RECEIVING_BODY;
        return 0;
    }
    
    __http_parse_header (line, request->headers);

    return 0;
}

/// Updates the HTTP request with the specified line, this will process it further.
int32_t http_request_update (http_request_t *request, char *line) {
    switch (request->state) {
    // When receiving the type.
    case HTTP_REQUEST_STATE_RECEIVING_TYPE:
        if (__http_request_update__type (request, line) != 0)
            return -1;
        break;
    // When receiving headers.
    case HTTP_REQUEST_STATE_RECEIVING_HEADERS:
        if (__http_request_update__headers (request, line) != 0)
            return -1;
        break;
    // Probally will not happen.
    default:
        break;
    }

    return 0;
}
