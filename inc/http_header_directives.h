#ifndef _HTTP_HEADER_DIRECTIVES_H
#define _HTTP_HEADER_DIRECTIVES_H

#include <stdint.h>

struct http_header_directive {
    struct http_header_directive   *prev;
    const char                     *key;
    const char                     *value;
    struct http_header_directive   *next;
};

typedef struct http_header_directive http_header_directive_t;

typedef struct {
    http_header_directive_t        *start;
    uint32_t                        count;
    http_header_directive_t        *end;
} http_header_directives_t;

#endif
