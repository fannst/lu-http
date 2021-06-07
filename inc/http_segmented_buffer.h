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

/*
    HTTP Line-Buffer: Basically singly-linked list with N-length strings which will be written to the server socket.
*/

#ifndef _HTTP_LINE_BUFFER_H
#define _HTTP_LINE_BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define HTTP_LINE_BUFFER_CREATE_LINE_FROM_STRING_FLAG__COPY     (1 << 0)

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////

struct http_segmented_buffer__segment {
    uint8_t *bytes;
    size_t written, total;
    struct http_segmented_buffer__segment *next, *prev;
};

typedef struct http_segmented_buffer__segment http_segmented_buffer__segment_t;

typedef struct {
    http_segmented_buffer__segment_t *start, *end;
    size_t segment_count;
} http_segmented_buffer_t;

///////////////////////////////////////////////////////////////////////////////
// HTTP Line Buffer
///////////////////////////////////////////////////////////////////////////////

/// Creates an new http segmented buffer.
http_segmented_buffer_t *http_segmented_buffer_create (void);

/// Frees an http segmented buffer.
void http_segmented_buffer_free (http_segmented_buffer_t **buffer);

/// Creates a new segment.
http_segmented_buffer__segment_t *http_segmented_buffer_segment_create_from_string (const char *string);

/// Creates a segment from specified size and string.
http_segmented_buffer__segment_t *http_segmented_buffer_segment_create (uint8_t *bytes, size_t len);

/// Appends one segment to the http line buffer.
int32_t http_segmented_buffer_append (http_segmented_buffer_t *buffer, http_segmented_buffer__segment_t *line);

/// Reads one segment from the http line buffer.
http_segmented_buffer__segment_t *http_segmented_buffer__get_end_segment (http_segmented_buffer_t *buffer);

/// Frees the latest segment from the http line buffer.
int32_t http_segmented_buffer__free_end_segment (http_segmented_buffer_t *buffer);

#endif
