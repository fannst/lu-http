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

#include "http_url.h"

/// Parses an URL.
int32_t http_url_parse (http_url_t *url, const char *raw) {
    size_t len = strlen (raw);

    // Checks if we're dealing with search parameters, if so read them from the
    //  url and store them in the reuslt URL.
    char *p = strrchr (raw, '?');
    if (p != NULL) {
        // Gets the length of the remaining part of the string, after which we allocate
        //  the required amount to store search in.
        size_t search_size = strlen (p);
        if ((url->search = malloc (search_size + 1)) == NULL)
            return -1;

        // Copies the search part of the string to the new url->search thingy.
        memcpy (url->search, p, search_size);
        url->search[search_size] = '\0';

        // Removes the search length from the final length to copy,
        //  this will leave us with the path only.
        len -= search_size;
    }

    // Allocates the memory to store the path in.
    if ((url->path = malloc (len + 1)) == NULL) {
        if (url->search != NULL)
            free (url->search);
        return -1;
    }
    
    // Copies the path into the newly allocated memory.
    memcpy (url->path, raw, len);
    url->path[len] = '\0';
    
    return 0;
}

/// Frees an URL.
void http_url_free (http_url_t *url) {
    if (url->path != NULL)
        free (url->path);
    if (url->search != NULL)
        free (url->search);
}
