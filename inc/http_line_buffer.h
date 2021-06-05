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

struct __http_line_buffer__line {
    char *string;
    size_t written, total;

    struct __http_line_buffer__line *next, *prev;
};

typedef struct __http_line_buffer__line __http_line_buffer__line_t;

typedef struct {
    __http_line_buffer__line_t *start, *end;
    size_t line_count;
} __http_line_buffer_t;

///////////////////////////////////////////////////////////////////////////////
// HTTP Line Buffer
///////////////////////////////////////////////////////////////////////////////

/// Creates an new http line buffer.
__http_line_buffer_t *http_line_buffer_create (void);

/// Frees an http line buffer.
void http_line_buffer_free (__http_line_buffer_t **buffer);

/// Creates a new line.
__http_line_buffer__line_t *http_line_buffer_line_create_from_string (const char *string);

/// Creates a line from specified size and string.
__http_line_buffer__line_t *http_line_buffer_line_create (char *string, size_t len);

/// Appends one line to the http line buffer.
int32_t http_line_buffer_append (__http_line_buffer_t *buffer, __http_line_buffer__line_t *line);

/// Reads one line from the http line buffer.
__http_line_buffer__line_t *http_line_buffer_get_latest_line (__http_line_buffer_t *buffer);

/// Frees the latest line from the http line buffer.
int32_t http_line_buffer_free_latest_line (__http_line_buffer_t *buffer);

#endif
