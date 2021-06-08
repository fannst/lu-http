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

#ifndef _HTTP_ACCEPT_RANGE_H
#define _HTTP_ACCEPT_RANGE_H

#include <stdlib.h>

typedef enum {
    HTTP_ACCEPT_RANGE_BYTES = 0,
    HTTP_ACCEPT_RANGE_NONE
} http_range_unit_t;

typedef struct {
    http_range_unit_t       unit;
    size_t                 *ranges;
} http_range_t;

/// Gets the string version of the accept range.
const char *http_accept_range_to_string (http_range_unit_t range);

#endif
