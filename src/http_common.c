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

#include "http_common.h"

/// Prints an error code with custom message.
void errc_print (const char *message) {
    fprintf (stderr, "[LUHTTP ERR] %s: %d -> %s\n", message, errno, strerror (errno));
}

/// Compares two strings case insensitive.
bool strcicmp (const char *a, const char *b) {
    for (;;) {
        if (tolower (*a) != tolower (*b))
            return false;

        ++a;
        ++b;

        if (*a == '\0' && *b == '\0')
            return true;
        else if (*a == '\0' || *b == '\0')
            return false;
    }
}

/// Gets the extension from an file.
const char *path_get_ext (const char *path) {
    return strrchr (path, '.');
}
