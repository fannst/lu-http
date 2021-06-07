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

#include "http_method.h"

/// Parses an HTTP method from string.
http_method_t http_method_from_string (const char *str) {
    if (strcmp (str, "GET") == 0)
        return HTTP_METHOD_GET;
    else if (strcmp (str, "HEAD") == 0)
        return HTTP_METHOD_HEAD;
    else if (strcmp (str, "POST") == 0)
        return HTTP_METHOD_POST;
    else if (strcmp (str, "PUT") == 0)
        return HTTP_METHOD_PUT;
    else if (strcmp (str, "DELETE") == 0)
        return HTTP_METHOD_DELETE;
    else if (strcmp (str, "TRACE") == 0)
        return HTTP_METHOD_TRACE;
    else if (strcmp (str, "OPTIONS") == 0)
        return HTTP_METHOD_OPTIONS;
    else if (strcmp (str, "CONNECT") == 0)
        return HTTP_METHOD_CONNECT;
    else if (strcmp (str, "PATCH") == 0)
        return HTTP_METHOD_PATCH;

    return HTTP_METHOD_INVALID;
}

/// Returns the string version of HTTP method.
const char *http_method_to_string (http_method_t method) {
    switch (method) {
    case HTTP_METHOD_GET:
        return "GET";
    case HTTP_METHOD_HEAD:
        return "HEAD";
    case HTTP_METHOD_POST:
        return "POST";
    case HTTP_METHOD_PUT:
        return "PUT";
    case HTTP_METHOD_DELETE:
        return "DELETE";
    case HTTP_METHOD_TRACE:
        return "TRACE";
    case HTTP_METHOD_OPTIONS:
        return "OPTIONS";
    case HTTP_METHOD_CONNECT:
        return "CONNECT";
    case HTTP_METHOD_PATCH:
        return "PATCH";
    case HTTP_METHOD_INVALID:
    default:
        return NULL;
    }
}
