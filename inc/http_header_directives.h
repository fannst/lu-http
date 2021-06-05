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

#ifndef _HTTP_HEADER_DIRECTIVES_H
#define _HTTP_HEADER_DIRECTIVES_H

#include <stdint.h>

struct http_header_directive {
    struct http_header_directive   *prev;
    const char                     *key;
    const char                     *value;
    struct http_header_directive   *next;
};

typedef struct http_header_directive http_header_directive_t;

typedef struct {
    http_header_directive_t        *start;
    uint32_t                        count;
    http_header_directive_t        *end;
} http_header_directives_t;

#endif
