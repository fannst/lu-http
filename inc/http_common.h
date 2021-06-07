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

#ifndef _HTTP_COMMON_H
#define _HTTP_COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

/// Prints an error code with custom message.
void errc_print (const char *message);

/// Compares two strings case insensitive.
bool strcicmp (const char *a, const char *b);

/// Gets the extension from an file.
const char *path_get_ext (const char *path);

#endif
