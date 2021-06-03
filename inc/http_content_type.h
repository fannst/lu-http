#ifndef _HTTP_CONTENT_TYPE_H
#define _HTTP_CONTENT_TYPE_H

#include <stdint.h>
#include <string.h>

typedef enum {
    HTTP_CONTENT_TYPE_TEXT_PLAIN,
    HTTP_CONTENT_TYPE_TEXT_HTML
} http_content_type_t;

/// Gets the string version of an HTTP content type.
const char *http_content_type_to_string (http_content_type_t type);

#endif
