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

#include "http_version.h"

/// Parses an HTTP version from string.
http_version_t http_version_from_string (const char *str) {
    if (strcmp (str, "HTTP/1.0") == 0)
        return HTTP_VERSION_1_0;
    else if (strcmp (str, "HTTP/1.1") == 0)
        return HTTP_VERSION_1_1;
    else if (strcmp (str, "HTTP/2") == 0)
        return HTTP_VERSION_2;
    else if (strcmp (str, "HTTP/3") == 0)
        return HTTP_VERSION_3;
    else
        return HTTP_VERSION_INVALID;
}

/// Returns the string version of version.
const char *http_version_to_string (http_version_t version) {
    switch (version) {
    case HTTP_VERSION_1_0:
        return "HTTP/1.0";
    case HTTP_VERSION_1_1:
        return "HTTP/1.1";
    case HTTP_VERSION_2:
        return "HTTP/2";
    case HTTP_VERSION_3:
        return "HTTP/3";
    case HTTP_VERSION_INVALID:
    default:
        return NULL;
    }
}
