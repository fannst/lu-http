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

#ifndef _HTTP_URL_H
#define _HTTP_URL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

typedef struct {
    char           *path;
    char           *search;
} http_url_t;

/// Parses an URL.
int32_t http_url_parse (http_url_t *url, const char *raw);

/// Frees an URL.
void http_url_free (http_url_t *url);

#endif
