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

#ifndef _HTTP_CONTENT_TYPE_H
#define _HTTP_CONTENT_TYPE_H

#include <stdint.h>
#include <string.h>

typedef enum {
    HTTP_CONTENT_TYPE_TEXT_PLAIN = 0,
    HTTP_CONTENT_TYPE_TEXT_HTML,
    HTTP_CONTENT_TYPE_TEXT_CSS,
    HTTP_CONTENT_TYPE_TEXT_JS,
    HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED,
    HTTP_CONTENT_TYPE_APPLICATION_JSON,
    HTTP_CONTENT_TYPE_UNKNOWN
} http_content_type_t;

/// Gets the string version of an HTTP content type.
const char *http_content_type_to_string (http_content_type_t type);

/// Gets an HTTP content type from string.
http_content_type_t http_content_type_from_string (const char *str);

#endif
