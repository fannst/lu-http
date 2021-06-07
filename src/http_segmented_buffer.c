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

#include "http_segmented_buffer.h"

///////////////////////////////////////////////////////////////////////////////
// HTTP segment Buffer
///////////////////////////////////////////////////////////////////////////////

/// Creates an new segmented buffer.
http_segmented_buffer_t *http_segmented_buffer_create (void) {
    return (http_segmented_buffer_t *) calloc (1, sizeof (http_segmented_buffer_t));
}

/// Frees an http segmented buffer.
void http_segmented_buffer_free (http_segmented_buffer_t **buffer) {
    // Checks if there are any elements left, if so remove free them.
    http_segmented_buffer__segment_t *segment = (*buffer)->start;
    while (segment != NULL) {
        // Stores the next segment as temp, and frees it.
        http_segmented_buffer__segment_t *next = segment->next;
        free (segment->bytes);
        free (segment);

        // Goes to the next segment.
        segment = next;
    }
    
    // Frees the actual segment buffer.
    free (*buffer);
    *buffer = NULL;
}

/// Creates a new segment.
http_segmented_buffer__segment_t *http_segmented_buffer_segment_create_from_string (const char *string) {
    // Allocates the memory for the segment structure.
    http_segmented_buffer__segment_t *res = (http_segmented_buffer__segment_t *) calloc (1, sizeof (http_segmented_buffer__segment_t));
    if (res == NULL)
        return NULL;

    // Gets the length of the segment.
    res->total = strlen (string);

    // Allocates the memory for the copied string, and copies the string
    //  to the target memory.
    res->bytes = (uint8_t *) malloc (res->total + 1);
    if (res->bytes == NULL) {
        free (res);
        return NULL;
    }

    memcpy (res->bytes, string, res->total + 1);

    // Returns the result.
    return res;
}

/// Creates a segment from specified size and string.
http_segmented_buffer__segment_t *http_segmented_buffer_segment_create (uint8_t *bytes, size_t len) {
    http_segmented_buffer__segment_t *res = (http_segmented_buffer__segment_t *) calloc (1, sizeof (http_segmented_buffer__segment_t));
    if (res == NULL)
        return NULL;

    res->bytes = bytes;
    res->total = len;

    return res;
}

/// Appends one segment to the http segment buffer.
int32_t http_segmented_buffer_append (http_segmented_buffer_t *buffer, http_segmented_buffer__segment_t *segment) {
    // Checks if the buffer was empty or not, if it's empty
    //  set the start and end to specified segment, else insert
    //  it at start.
    if (buffer->segment_count == 0) {
        buffer->start = buffer->end = segment;
    } else {
        segment->next = buffer->start;
        buffer->start->prev = segment;

        buffer->start = segment;
    }

    // Increments the segment count, since we're adding a new one.
    ++buffer->segment_count;

    return 0;
}

/// Reads one segment from the http segment buffer.
http_segmented_buffer__segment_t *http_segmented_buffer__get_end_segment (http_segmented_buffer_t *buffer) {
    return buffer->end;
}

/// Frees the latest segment from the http segment buffer.
int32_t http_segmented_buffer__free_end_segment (http_segmented_buffer_t *buffer) {
    if (buffer->segment_count == 0)
        return -1; 
    else if (buffer->segment_count == 1) {
        free (buffer->end->bytes);
        free (buffer->end);

        buffer->end = buffer->start = NULL;
    } else {
        buffer->end->prev->next = NULL;
        
        http_segmented_buffer__segment_t *prev = buffer->end->prev;

        free (buffer->end->bytes);
        free (buffer->end);

        buffer->end = prev;
    }

    // Decrements the number of segments.
    --buffer->segment_count;

    return 0;
}
