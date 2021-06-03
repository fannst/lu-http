#ifndef _HTTP_HEADER_H
#define _HTTP_HEADER_H

#define __HTTP_HEADER_DEBUG

#ifdef __HTTP_HEADER_DEBUG
#define HTTP_HEADER_DEBUG_ONLY(A) A
#else
#define HTTP_HEADER_DEBUG_ONLY(A)
#endif

#define HTTP_HEADER_INSERT_FLAG_REPLACE         (1 << 0)
#define HTTP_HEADER_INSERT_FLAG_BEGIN           (1 << 1)
#define HTTP_HEADER_INSERT_FLAG_END             (1 << 2)
#define HTTP_HEADER_INSERT_FLAG_COPY_KEY        (1 << 3)
#define HTTP_HEADER_INSERT_FLAG_COPY_VALUE      (1 << 4)

#define HTTP_HEADER_FLAG_FREE_KEY               (1 << 0)
#define HTTP_HEADER_FLAG_FREE_VALUE             (1 << 1)

#define HTTP_HEADERS_WALK_CTX_FLAG_FORWARD      (1 << 0)
#define HTTP_HEADERS_WALK_CTX_FLAG_BACKWARD     (1 << 0)

#define HTTP_PARSE_HEADER_FLAG_KEEP_INTACT      (1 << 0)

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////

struct http_header {
    struct http_header *prev;
    uint32_t            flags;  
    const char         *key;
    const char         *value;
    struct http_header *next;
};

typedef struct http_header http_header_t;

typedef struct {
    http_header_t  *start;
    http_header_t  *end;
    uint32_t        count;
} http_headers_t;

typedef struct {
    uint32_t        flags;
    http_headers_t *headers;
    http_header_t  *next;
} http_headers_walk_ctx_t;

typedef void (*http_header_to_string_no_collapse_cb) (const char *, void *);

///////////////////////////////////////////////////////////////////////////////
// Header Methods
///////////////////////////////////////////////////////////////////////////////

/// Creates new HTTP headers structure.
http_headers_t *http_headers_new (void);

/// Copies an HTTP header, and stores the pointer in p. 
int32_t __http_headers_insert_copy_val (const char **p);

/// Adds the first item to the doubly-linked-list.
void __http_header_insert_first (http_headers_t *headers, http_header_t *header);

/// Inserts an header to the end of the structure.
void __http_header_insert_end (http_headers_t *headers, http_header_t *header);

/// Inserts an header at the start of the structure.
void __http_header_insert_start (http_headers_t *headers, http_header_t *header);

///////////////////////////////////////////////////////////////////////////////
// Header Walking Methods
///////////////////////////////////////////////////////////////////////////////

/// Frees an HTTP header walk context.
void http_header_walk_ctx_free (http_headers_walk_ctx_t **ctx);

/// Starts walking the doubly-linked-list.
http_headers_walk_ctx_t *http_headers_walk_ctx_new (http_headers_t *headers, uint32_t flags);

/// Walks to the next element in the doubly-linked-list.
http_header_t *__http_headers_walk_next (http_headers_walk_ctx_t *ctx);

/// Insert header to HTTP headers structure.
int32_t http_headers_insert (http_headers_t *headers, const char *key, const char *value, uint32_t flags);

/// Frees an HTTP header.
void __http_header_free (http_header_t **header);

/// Frees existing headers structure.
int32_t http_headers_free (http_headers_t **headers);

/// Adds all headers from structure to another one.
int32_t http_headers_add_all (http_headers_t *target, http_headers_t *from);

///////////////////////////////////////////////////////////////////////////////
// To String Methods
///////////////////////////////////////////////////////////////////////////////

/// Loops over the headers and creates the non-collpased string versions.
int32_t http_headers_to_string_no_collapse (http_headers_t *headers, http_header_to_string_no_collapse_cb cb, void *u);

///////////////////////////////////////////////////////////////////////////////
// Parsing
///////////////////////////////////////////////////////////////////////////////

/// Removes excessive suffix whitespace.
void __http_header_remove_suffix_ws (char *p);

/// Removes excessive prefix whitespace.
void __http_header_remove_prefix_ws (char *p);

/// Gets the tidy version of the specified header key, eg removing extra whitespace.
void __http_header_tidy_key (char *key);

/// Parses a single header.
int32_t __http_parse_header (char *header_key, http_headers_t *target);

/// Parses an list of headers until empty line is detected.
http_headers_t *parse_http_headers (char *raw, uint32_t flags);

#endif
