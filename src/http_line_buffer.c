#include "http_line_buffer.h"

///////////////////////////////////////////////////////////////////////////////
// HTTP Line Buffer
///////////////////////////////////////////////////////////////////////////////

/// Creates an new line buffer.
__http_line_buffer_t *http_line_buffer_create (void) {
    return (__http_line_buffer_t *) calloc (1, sizeof (__http_line_buffer_t));
}

/// Frees an http line buffer.
void http_line_buffer_free (__http_line_buffer_t **buffer) {
    // Checks if there are any elements left, if so remove free them.
    __http_line_buffer__line_t *line = (*buffer)->start;
    while (line != NULL) {
        // Stores the next line as temp, and frees it.
        __http_line_buffer__line_t *next = line->next;
        free (line->string);
        free (line);

        // Goes to the next line.
        line = next;
    }
    
    // Frees the actual line buffer.
    free (*buffer);
    *buffer = NULL;
}

/// Creates a new line.
__http_line_buffer__line_t *http_line_buffer_line_create_from_string (const char *string) {
    // Allocates the memory for the line structure.
    __http_line_buffer__line_t *res = (__http_line_buffer__line_t *) calloc (1, sizeof (__http_line_buffer__line_t));
    if (res == NULL)
        return NULL;

    // Gets the length of the line.
    res->total = strlen (string);

    // Allocates the memory for the copied string, and copies the string
    //  to the target memory.
    res->string = (char *) malloc (res->total + 1);
    if (res->string == NULL) {
        free (res);
        return NULL;
    }

    memcpy (res->string, string, res->total + 1);

    // Returns the result.
    return res;
}

/// Creates a line from specified size and string.
__http_line_buffer__line_t *http_line_buffer_line_create (char *string, size_t len) {
    __http_line_buffer__line_t *res = (__http_line_buffer__line_t *) calloc (1, sizeof (__http_line_buffer__line_t));
    if (res == NULL)
        return NULL;

    res->string = string;
    res->total = len;

    return res;
}

/// Appends one line to the http line buffer.
int32_t http_line_buffer_append (__http_line_buffer_t *buffer, __http_line_buffer__line_t *line) {
    // Checks if the buffer was empty or not, if it's empty
    //  set the start and end to specified line, else insert
    //  it at start.
    if (buffer->line_count == 0) {
        buffer->start = buffer->end = line;
    } else {
        line->next = buffer->start;
        buffer->start->prev = line;

        buffer->start = line;
    }

    // Increments the line count, since we're adding a new one.
    ++buffer->line_count;

    return 0;
}

/// Reads one line from the http line buffer.
__http_line_buffer__line_t *http_line_buffer_get_latest_line (__http_line_buffer_t *buffer) {
    return buffer->end;
}

/// Frees the latest line from the http line buffer.
int32_t http_line_buffer_free_latest_line (__http_line_buffer_t *buffer) {
    if (buffer->line_count == 0)
        return -1; 
    else if (buffer->line_count == 1) {
        free (buffer->end->string);
        free (buffer->end);

        buffer->end = buffer->start = NULL;
    } else {
        buffer->end->prev->next = NULL;
        
        __http_line_buffer__line_t *prev = buffer->end->prev;

        free (buffer->end->string);
        free (buffer->end);

        buffer->end = prev;
    }

    // Decrements the number of lines.
    --buffer->line_count;

    return 0;
}
