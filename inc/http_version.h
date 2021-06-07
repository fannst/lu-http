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

#ifndef _HTTP_VERSION_H
#define _HTTP_VERSION_H

#include <stdlib.h>
#include <string.h>

typedef enum {
    HTTP_VERSION_INVALID = 0,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2,
    HTTP_VERSION_3
} http_version_t;

/// Parses an HTTP version from string.
http_version_t http_version_from_string (const char *str);

/// Returns the string version of version.
const char *http_version_to_string (http_version_t version);

#endif
