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

#ifndef _HTTP_CODE_H
#define _HTTP_CODE_H

#include <stdlib.h>

typedef enum {
    /* 1xx Informational */
    HTTP_CODE__INFORMATIONAL__CONTINUE = 100,
    HTTP_CODE__INFORMATIONAL__SWITCHING_PROTO = 101,
    HTTP_CODE__INFORMATIONAL__EARLY_HINTS = 103,
    /* 2xx Sucess */
    HTTP_CODE__SUCCESS__OK = 200,
    HTTP_CODE__SUCCESS__CREATED = 201,
    HTTP_CODE__SUCCESS__ACCEPTED = 202,
    HTTP_CODE__SUCCESS__NON_AUTHORATIVE_INFO = 203,
    HTTP_CODE__SUCCESS__NO_CONTENT = 204,
    HTTP_CODE__SUCCESS__RESET_CONTENT = 205,
    HTTP_CODE__SUCCESS__PARTIAL_CONTENT = 206,
    HTTP_CODE__SUCCESS__IM_USED = 226,
    /* 3xx Redirect */
    HTTP_CODE__REDIRECT__MULTIPLE_CHOICES = 300,
    HTTP_CODE__REDIRECT__MOVED_PERMANENTLY = 301,
    HTTP_CODE__REDIRECT__FOUND = 302,
    HTTP_CODE__REDIRECT__SEE_OTHER = 303,
    HTTP_CODE__REDIRECT__NOT_MODIFIED = 304,
    HTTP_CODE__REDIRECT__USE_PROXY = 305,
    HTTP_CODE__REDIRECT__SWITCHING_PROXY = 306,
    HTTP_CODE__REDIRECT__TEMP_REDIRECT = 307,
    HTTP_CODE__REDIRECT__PERM_REDIRECT = 308,
    /* 4xx Client Errors */
    HTTP_CODE__CLIENT_ERROR__BAD_REQUEST = 400,
    HTTP_CODE__CLIENT_ERROR__UNAUTHORIZED = 401,
    HTTP_CODE__CLIENT_ERROR__PAYMENT_REQUIRED = 402,
    HTTP_CODE__CLIENT_ERROR__FORBIDDEN = 403,
    HTTP_CODE__CLIENT_ERROR__NOT_FOUND = 404,
    HTTP_CODE__CLIENT_ERROR__METHOD_NOT_ALLOWED = 405,
    HTTP_CODE__CLIENT_ERROR__NOT_ACCEPTABLE = 406,
    HTTP_CODE__CLIENT_ERROR__REQUEST_TIMEOUT = 408,
    HTTP_CODE__CLIENT_ERROR__CONFLICT = 409,
    HTTP_CODE__CLIENT_ERROR__GONE = 410,
    HTTP_CODE__CLIENT_ERROR__LENGTH_REQUIRED = 411,
    HTTP_CODE__CLIENT_ERROR__PRECONDITION_FAILED = 412,
    HTTP_CODE__CLIENT_ERROR__PAYLOAD_TOO_LARGE = 413,
    HTTP_CODE__CLIENT_ERROR__URI_TOO_LONG = 414,
    HTTP_CODE__CLIENT_ERROR__UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_CODE__CLIENT_ERROR__RANGE_NOT_SATISFIABLE = 416,
    HTTP_CODE__CLIENT_ERROR__EXPECTATION_FAILED = 417,
    HTTP_CODE__CLIENT_ERROR__MISDIRECTED_REQUEST = 321,
    HTTP_CODE__CLIENT_ERROR__TOO_EARLY = 425,
    HTTP_CODE__CLIENT_ERROR__UPGRADE_REQUIRED = 426,
    HTTP_CODE__CLIENT_ERROR__PRECONDITION_REQUIRED = 428,
    HTTP_CODE__CLIENT_ERROR__TOO_MANY_REQUESTS = 429,
    HTTP_CODE__CLIENT_ERROR__REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_CODE__CLIENT_ERROR__UNAVAILABLE_FOR_LEGAL_REASONS = 451,
    /* 5xx Server Errors */
    HTTP_CODE__SERVER_ERROR__INTERNAL_SERVER_ERROR = 500,
    HTTP_CODE__SERVER_ERROR__NOT_IMPLEMENTED = 501,
    HTTP_CODE__SERVER_ERROR__BAD_GATEWAY = 502,
    HTTP_CODE__SERVER_ERROR__SERVICE_UNAVAILABLE = 503,
    HTTP_CODE__SERVER_ERROR__GATEWAY_TIMEOUT = 504,
    HTTP_CODE__SERVER_ERROR__HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_CODE__SERVER_ERROR__NOT_EXTENDED = 510
} http_code_t;

/// Gets the message associated with HTTP code.
const char *http_code_get_message (http_code_t code);

#endif