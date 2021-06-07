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


#include "http_code.h"

/// Gets the message associated with HTTP code.
const char *http_code_get_message (http_code_t code) {
    switch (code) {
    case HTTP_CODE__INFORMATIONAL__CONTINUE:
        return "Continue";
    case HTTP_CODE__INFORMATIONAL__SWITCHING_PROTO:
        return "Switching Protocols";
    case HTTP_CODE__INFORMATIONAL__EARLY_HINTS:
        return "Early Hints";

    case HTTP_CODE__SUCCESS__OK:
        return "OK";
    case HTTP_CODE__SUCCESS__CREATED:
        return "Created";
    case HTTP_CODE__SUCCESS__ACCEPTED:
        return "Accepted";
    case HTTP_CODE__SUCCESS__NON_AUTHORATIVE_INFO:
        return "Non-Authoritative Information";
    case HTTP_CODE__SUCCESS__NO_CONTENT:
        return "No Content";
    case HTTP_CODE__SUCCESS__RESET_CONTENT:
        return "Reset Content";
    case HTTP_CODE__SUCCESS__PARTIAL_CONTENT:
        return "Partial Content";
    case HTTP_CODE__SUCCESS__IM_USED:
        return "IM Used";

    case HTTP_CODE__REDIRECT__MULTIPLE_CHOICES:
        return "Multiple Choices";
    case HTTP_CODE__REDIRECT__MOVED_PERMANENTLY:
        return "Moved Permanently";
    case HTTP_CODE__REDIRECT__FOUND:
        return "Found";
    case HTTP_CODE__REDIRECT__SEE_OTHER:
        return "See Other";
    case HTTP_CODE__REDIRECT__NOT_MODIFIED:
        return "Not Modified";
    case HTTP_CODE__REDIRECT__USE_PROXY:
        return "Use Proxy";
    case HTTP_CODE__REDIRECT__SWITCHING_PROXY:
        return "Switch Proxy";
    case HTTP_CODE__REDIRECT__TEMP_REDIRECT:
        return "Temporary Redirect";
    case HTTP_CODE__REDIRECT__PERM_REDIRECT:
        return "Permanent Redirect";

    case HTTP_CODE__CLIENT_ERROR__BAD_REQUEST:
        return "Bad Request";
    case HTTP_CODE__CLIENT_ERROR__UNAUTHORIZED:
        return "Unauthorized";
    case HTTP_CODE__CLIENT_ERROR__PAYMENT_REQUIRED:
        return "Payment Required";
    case HTTP_CODE__CLIENT_ERROR__FORBIDDEN:
        return "Forbidden";
    case HTTP_CODE__CLIENT_ERROR__NOT_FOUND:
        return "Not Found";
    case HTTP_CODE__CLIENT_ERROR__METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case HTTP_CODE__CLIENT_ERROR__NOT_ACCEPTABLE:
        return "Not Acceptable";
    case HTTP_CODE__CLIENT_ERROR__REQUEST_TIMEOUT:
        return "Request Timeout";
    case HTTP_CODE__CLIENT_ERROR__CONFLICT:
        return "Conflict";
    case HTTP_CODE__CLIENT_ERROR__GONE:
        return "Gone";
    case HTTP_CODE__CLIENT_ERROR__LENGTH_REQUIRED:
        return "Length Required";
    case HTTP_CODE__CLIENT_ERROR__PRECONDITION_FAILED:
        return "Precondition Failed";
    case HTTP_CODE__CLIENT_ERROR__PAYLOAD_TOO_LARGE:
        return "Payload Too Large";
    case HTTP_CODE__CLIENT_ERROR__URI_TOO_LONG:
        return "URI Too Long";
    case HTTP_CODE__CLIENT_ERROR__UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type";
    case HTTP_CODE__CLIENT_ERROR__RANGE_NOT_SATISFIABLE:
        return "Range Not Satisfiable";
    case HTTP_CODE__CLIENT_ERROR__EXPECTATION_FAILED:
        return "Expectation Failed";
    case HTTP_CODE__CLIENT_ERROR__MISDIRECTED_REQUEST:
        return "Misdirected Request";
    case HTTP_CODE__CLIENT_ERROR__TOO_EARLY:
        return "Too Early";
    case HTTP_CODE__CLIENT_ERROR__UPGRADE_REQUIRED:
        return "Upgrade Required";
    case HTTP_CODE__CLIENT_ERROR__PRECONDITION_REQUIRED:
        return "Precondition Required";
    case HTTP_CODE__CLIENT_ERROR__TOO_MANY_REQUESTS:
        return "Too Many Requests";
    case HTTP_CODE__CLIENT_ERROR__REQUEST_HEADER_FIELDS_TOO_LARGE:
        return "Request Header Fields Too Large";
    case HTTP_CODE__CLIENT_ERROR__UNAVAILABLE_FOR_LEGAL_REASONS:
        return "Unavailable For Legal Reasons";

    case HTTP_CODE__SERVER_ERROR__INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    case HTTP_CODE__SERVER_ERROR__NOT_IMPLEMENTED:
        return "Not Implemented";
    case HTTP_CODE__SERVER_ERROR__BAD_GATEWAY:
        return "Bad Gateway";
    case HTTP_CODE__SERVER_ERROR__SERVICE_UNAVAILABLE:
        return "Service Unavailable";
    case HTTP_CODE__SERVER_ERROR__GATEWAY_TIMEOUT:
        return "Gateway Timeout";
    case HTTP_CODE__SERVER_ERROR__HTTP_VERSION_NOT_SUPPORTED:
        return "HTTP Version Not Supported";
    case HTTP_CODE__SERVER_ERROR__NOT_EXTENDED:
        return "Not Extended";

    default:
        return NULL;
    }
}