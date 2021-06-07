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
    else
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

/// Creates an new HTTP request.
http_request_t *http_request_create (void) {
    // Allocates the memory for the request.
    http_request_t *res = (http_request_t *) calloc (1, sizeof (http_request_t));
    if (res == NULL)
        return NULL;

    // Allocates the line buffer.
    res->body = http_segmented_buffer_create ();
    if (res->body == NULL) {
        free (res);
        return NULL;
    }
    
    // Allocates the new headers.
    res->headers = http_headers_new ();
    if (res->headers == NULL) {
        http_segmented_buffer_free (&res->body);
        free (res);
        return NULL;
    }

    res->state = HTTP_REQUEST_STATE_RECEIVING_TYPE;

    return res;
}

/// Frees an HTTP request.
int32_t http_request_free (http_request_t **req) {
    if (http_headers_free (&((*req)->headers)) != 0)
        return -1;
    
    if ((*req)->url != NULL)
        free ((*req)->url);


    free (*req);
    *req = NULL;
    
    return 0;
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
    printf ("- Content Type: %s\r\n", http_content_type_to_string (request->content_type));
    printf ("- Content Length: %lu\r\n", request->expected_body_size);
    printf ("- Headers:\r\n");
    http_headers_to_string_no_collapse (request->headers, __http_request_print__header_method, NULL);

    if (request->content_type == HTTP_CONTENT_TYPE_TEXT_PLAIN ||
        request->content_type == HTTP_CONTENT_TYPE_APPLICATION_JSON ||
        request->content_type == HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED)
    {
        printf ("- Body (%lu):\r\n", request->body->segment_count);

        http_segmented_buffer__segment_t *segment = request->body->start;
        while (segment != NULL) {
            printf ("%s\r\n", (char *) segment->bytes);
            segment = segment->next;
        }
    }
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

    // Sets the next state, and returns 0.
    request->state = HTTP_REQUEST_STATE_RECEIVING_HEADERS;
    return 0;
}

/// Updates the HTTP request when header state is specified.
int32_t __http_request_update__headers (http_request_t *request, char *line) {
    // Checks if we're not dealing with an empty line, if so parse the header
    //  and return 0 (if no errors).
    if (!(strcmp (line, "") == 0)) {
        if (__http_parse_header (line, request->headers) != 0)
            return -1;
        
        return 0;
    }
    
    // Since the HTTP protocol is one big fucking inneficient mess, there are no true indicators
    //  for request bodies, and that's why we're required to check the content-type and content-length
    //  to check if any body may possibly be supplied.

    http_header_t *header = NULL;
    if ((header = http_headers_find_by_key (request->headers, "content-type")) != NULL) {
        request->content_type = http_content_type_from_string (header->value);
    }

    if ((header = http_headers_find_by_key (request->headers, "content-length")) != NULL) {
        request->expected_body_size = (size_t) atol (header->value);
    }

    // Checks if we're going to read an body or not.
    if (request->content_type != HTTP_CONTENT_TYPE_UNKNOWN && request->expected_body_size > 0)
        request->state = HTTP_REQUEST_STATE_RECEIVING_BODY;
    else
        request->state = HTTP_REQUEST_STATE_DONE;

    return 0;
}

/// Updates the HTTP request with the specified data, this will process it further.
int32_t http_request_update (http_request_t *request, uint8_t *data) {
    switch (request->state) {
    // When receiving the type.
    case HTTP_REQUEST_STATE_RECEIVING_TYPE:
        if (__http_request_update__type (request, (char *) data) != 0)
            return -1;
        break;
    // When receiving headers.
    case HTTP_REQUEST_STATE_RECEIVING_HEADERS:
        if (__http_request_update__headers (request, (char *) data) != 0)
            return -1;
        break;
    // When receiving body.
    case HTTP_REQUEST_STATE_RECEIVING_BODY:
        break;
    // Probally will not happen.
    default:
        break;
    }

    return 0;
}
