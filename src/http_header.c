#include "http_header.h"

///////////////////////////////////////////////////////////////////////////////
// Header Methods
///////////////////////////////////////////////////////////////////////////////

/// Creates new HTTP headers structure.
http_headers_t *http_headers_new (void) {
    http_headers_t *headers = (http_headers_t *) malloc (sizeof (http_headers_t));
    if (headers == NULL)
        return NULL;

    headers->end = headers->start = NULL;
    headers->count = 0;
    
    return headers;
}

/// Copies an HTTP header, and stores the pointer in p. 
int32_t __http_headers_insert_copy_val (const char **p) {
    const char *temp;
    size_t len;

    // Gets the length of the original value, and allocates the required memory.
    len = strlen (*p) + 1;
    temp = (const char *) malloc (len);
    if (temp == NULL)
        return -1;

    // Copy the original data into the new memory, and replace
    //  the original pointer.
    memcpy ((void *) temp, (void *) *p, len);
    *p = temp;

    return 0;
}

/// Adds the first item to the doubly-linked-list.
void __http_header_insert_first (http_headers_t *headers, http_header_t *header) {
    header->prev = NULL;
    header->next = NULL;
        
    headers->start = header;
    headers->end = header;
}

/// Inserts an header to the end of the structure.
void __http_header_insert_end (http_headers_t *headers, http_header_t *header) {
    ++headers->count;
    
    // Checks if the doubly-linked-list is empty, if so, 
    //  simply add it to end and start.
    if (headers->count == 1) {
        __http_header_insert_first (headers, header);
        return;
    }

    // Since the double-linked-list is not empty, insert it at the end
    //  and add the previous one as pointer.

    headers->end->next = header;
    
    header->prev = headers->end;
    header->next = NULL;

    headers->end = header;
}

/// Inserts an header at the start of the structure.
void __http_header_insert_start (http_headers_t *headers, http_header_t *header) {
    ++headers->count;

    // Checks if the doubly-linked-list is empty, if so, 
    //  simply add it to end and start.
    if (headers->count == 1) {
        __http_header_insert_first (headers, header);
        return;
    }

    // Since the double-linked-list is not empty, insert it at the start,
    //  and reference the first one as next.

    headers->start->prev = header;
    
    header->next = headers->start;
    header->prev = NULL;
    
    headers->start = header;
}

///////////////////////////////////////////////////////////////////////////////
// Header Walking Methods
///////////////////////////////////////////////////////////////////////////////

/// Frees an HTTP header walk context.
void http_header_walk_ctx_free (http_headers_walk_ctx_t **ctx) {
    free (*ctx);
    *ctx = NULL;
}

/// Starts walking the doubly-linked-list.
http_headers_walk_ctx_t *http_headers_walk_ctx_new (http_headers_t *headers, uint32_t flags) {
    // Allocates the new walk structure, and checks if it succeeded.
    http_headers_walk_ctx_t *ctx = (http_headers_walk_ctx_t *) malloc (sizeof (http_headers_walk_ctx_t));
    if (ctx == NULL)
        return NULL;

    // Sets the flags and headers.
    ctx->flags = flags;
    ctx->headers = headers;

    // Checks which part of the structure to use as first next pointer.
    if (flags & HTTP_HEADERS_WALK_CTX_FLAG_FORWARD)
        ctx->next = headers->start;
    else if (flags & HTTP_HEADERS_WALK_CTX_FLAG_BACKWARD)
        ctx->next = headers->end;
    else
        http_header_walk_ctx_free (&ctx); // It will make the result NULL.

    return ctx;
}

/// Walks to the next element in the doubly-linked-list.
http_header_t *__http_headers_walk_next (http_headers_walk_ctx_t *ctx) {
    http_header_t *p = ctx->next;

    // If p NULL, just return NULL.
    if (p == NULL)
        return NULL;

    // Checks which direction to go, and sets the next pointer.
    if (ctx->flags & HTTP_HEADERS_WALK_CTX_FLAG_FORWARD)
        ctx->next = p->next;
    else if (ctx->flags & HTTP_HEADERS_WALK_CTX_FLAG_BACKWARD)
        ctx->next = p->prev;

    return p;
}

/// Insert header to HTTP headers structure.
int32_t http_headers_insert (http_headers_t *headers, const char *key, const char *value, uint32_t flags) {
    // Allocates the new HTTP header.
    http_header_t *header = malloc (sizeof (http_header_t));
    if (header == NULL)
        return -1;

    header->flags = 0;

    // Copies the memory if required.
    if (flags & HTTP_HEADER_INSERT_FLAG_COPY_KEY) {
        header->flags |= HTTP_HEADER_FLAG_FREE_KEY;
        if (__http_headers_insert_copy_val (&key) == -1) {
            free (header);
            return -1;
        }
    }
    
    if (flags & HTTP_HEADER_INSERT_FLAG_COPY_VALUE) {
        header->flags |= HTTP_HEADER_FLAG_FREE_VALUE;
        if (__http_headers_insert_copy_val (&value) == -1) {
            free (header);
            return -1;
        }
    }
    
    // Assigns the key and value.
    header->key = key;
    header->value = value;

    // Checks which insertion method to use.
    if (flags & HTTP_HEADER_INSERT_FLAG_BEGIN)
        __http_header_insert_end (headers, header);
    else if (flags & HTTP_HEADER_INSERT_FLAG_END)
        __http_header_insert_start (headers, header);
    else if (flags & HTTP_HEADER_INSERT_FLAG_REPLACE) {
        // TODO: Support this feature.
        free (header);
        return -1;
    }

    return 0;
}

/// Frees an HTTP header.
void __http_header_free (http_header_t **header) {
    if ((*header)->flags & HTTP_HEADER_FLAG_FREE_KEY)
        free ((void *) (*header)->key);
    if ((*header)->flags & HTTP_HEADER_FLAG_FREE_VALUE)
        free ((void *) (*header)->value);

    free (*header);
    *header = NULL;
}

/// Frees existing headers structure.
int32_t http_headers_free (http_headers_t **headers) {
    if (*headers == NULL)
        return 0;

    // Creates the header walk context.
    http_headers_walk_ctx_t *ctx = http_headers_walk_ctx_new (*headers, HTTP_HEADERS_WALK_CTX_FLAG_FORWARD);
    if (ctx == NULL)
        return -1;
    
    // Loops over all the headers, and frees the memory.
    http_header_t *header;
    while ((header = __http_headers_walk_next (ctx)) != NULL)
        __http_header_free (&header);

    // Frees the header walk context.
    http_header_walk_ctx_free (&ctx);

    // Frees the headers structure.
    free (*headers);
    *headers = NULL;

    return 0;
}

/// Adds all headers from structure to another one.
int32_t http_headers_add_all (http_headers_t *target, http_headers_t *from) {
     // Creates the header walk context.
    http_headers_walk_ctx_t *ctx = http_headers_walk_ctx_new (from, HTTP_HEADERS_WALK_CTX_FLAG_FORWARD);
    if (ctx == NULL)
        return -1;
    
    // Loops over all the headers, and frees the memory.
    http_header_t *header;
    while ((header = __http_headers_walk_next (ctx)) != NULL) {
        http_headers_insert (target, header->key, header->value, HTTP_HEADER_INSERT_FLAG_COPY_VALUE | HTTP_HEADER_INSERT_FLAG_COPY_KEY | HTTP_HEADER_INSERT_FLAG_END);
    }

    // Frees the header walk context.
    http_header_walk_ctx_free (&ctx);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// To String Methods
///////////////////////////////////////////////////////////////////////////////

/// Loops over the headers and creates the non-collpased string versions.
int32_t http_headers_to_string_no_collapse (http_headers_t *headers, http_header_to_string_no_collapse_cb cb, void *u) {
    // Creates the header walk context.
    http_headers_walk_ctx_t *ctx = http_headers_walk_ctx_new (headers, HTTP_HEADERS_WALK_CTX_FLAG_FORWARD);
    if (ctx == NULL)
        return -1;

    // Allocates the buffer.
    static const size_t buffer_size = 2048;
    char *buffer = (char *) malloc (buffer_size);
    if (buffer == NULL) {
        http_header_walk_ctx_free (&ctx);
        return -1;
    }

    // Loops over all the headers, and frees the memory.
    http_header_t *header;
    while ((header = __http_headers_walk_next (ctx)) != NULL) {
        snprintf (buffer, buffer_size, "%s: %s\r\n", header->key, header->value);
        cb ((const char *) buffer, u);
    }

    // Frees the header walk context.
    http_header_walk_ctx_free (&ctx);

    // Frees the buffer.
    free (buffer);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Parsing
///////////////////////////////////////////////////////////////////////////////

/// Removes excessive suffix whitespace.
void __http_header_remove_suffix_ws (char *p) {
    size_t size = strlen (p) + 1;
    while (p[size - 1] == ' ' || p[size - 1] == '\t') {
        p [size - 1] = '\0';
        size -= 1;
    }
}

/// Removes excessive prefix whitespace.
void __http_header_remove_prefix_ws (char *p) {
    size_t size = strlen (p) + 1;
    while (p[0] == ' ' || p[0] == '\t') {
        size -= 1;
        memmove (p, p + 1, size);
    }
}

/// Parses a single header.
int32_t __http_parse_header (char *header_key, http_headers_t *target) {
    // Attempts to get the position of the colon in the header
    //  the colon indicates key/value split.
    char *header_value = strchr (header_key, ':');
    if (header_value == NULL) {
        HTTP_HEADER_DEBUG_ONLY (printf ("Invalid Header (Could not find colon!): '%s'", header_key));
        return -1;
    }

    // Splits the given header in two parts, one holding the key, and the other one
    //  holding the header value.
    size_t header_value_start_index = (size_t) header_value - (size_t) header_key;
    header_key[header_value_start_index] = '\0';

    // Increments the header_value ptr, since it will now point to '\0',
    //  after doing this we make sure that the header value is not equal to the
    //  raw pointer.
    header_value += 1;
    if (header_value == header_key)
        return -1;

    // Cleans the header segments, removes excessive whitespace and tabs.
    __http_header_remove_prefix_ws (header_key);
    __http_header_remove_suffix_ws (header_key);

    __http_header_remove_prefix_ws (header_value);
    __http_header_remove_suffix_ws (header_value);

    // Allocates the memory required for the new header structure.
    if (http_headers_insert (target, header_key, header_value, HTTP_HEADER_INSERT_FLAG_BEGIN | HTTP_HEADER_INSERT_FLAG_COPY_KEY | HTTP_HEADER_INSERT_FLAG_COPY_VALUE) != 0)
        return -1;

    return 0;
}

/// Parses an list of headers until empty line is detected.
http_headers_t *parse_http_headers (char *raw, uint32_t flags) {
    static const char *crlf = "\r\n";
    char *save_ptr = NULL, *token = NULL;

    // Creates the new headers structure to store the parsed headers in.
    http_headers_t *headers = http_headers_new ();
    if (headers == NULL)
        return NULL;

    // Checks if the callee wants to keep the raw headers intact, if so
    //  we will copy the memory so the original won't be modified.
    if (flags & HTTP_PARSE_HEADER_FLAG_KEEP_INTACT) {
        // Allocates the memory for the header copy.
        char *t = (char *) malloc (strlen (raw) + 1);
        if (t == NULL) {
            http_headers_free (&headers);
            return NULL;
        }

        // Copies the raw headers into the new temp string, and replaces
        //  the raw header pointer with the newly allocated memory.
        strcpy (t, raw);
        raw = t;
    }

    // Starts looping over the header lines, and parsing them.
    token = strtok_r (raw, crlf, &save_ptr);
    do {
        if (__http_parse_header (token, headers) != 0) {
            http_headers_free (&headers);
            goto end;
        }
    } while ((token = strtok_r (NULL, crlf, &save_ptr)) != NULL);

    // Checks if we need to free some newly allocated memory, and if so
    //  free it. We're using the 'end' label to prevent duplicate code.
end:
    if (flags & HTTP_PARSE_HEADER_FLAG_KEEP_INTACT)
        free (raw);

    return headers;
}
