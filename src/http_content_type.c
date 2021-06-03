#include "http_content_type.h"

/// Gets the string version of an HTTP content type.
const char *http_content_type_to_string (http_content_type_t type) {
    switch (type) {
        case HTTP_CONTENT_TYPE_TEXT_HTML:
            return "text/html";
        case HTTP_CONTENT_TYPE_TEXT_PLAIN:
            return "text/plain";
        default:
            return NULL;
    }
}
