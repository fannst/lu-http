#include "http_common.h"

/// Prints an error code with custom message.
void errc_print (const char *message) {
    fprintf (stderr, "[LUHTTP ERR] %s: %d -> %s\n", message, errno, strerror (errno));
}