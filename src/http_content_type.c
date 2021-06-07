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

#include "http_content_type.h"

const char *http_content_type_text_html_str = "text/html";
const char *http_content_type_text_plain_str = "text/plain";
const char *http_content_type_text_css_str = "text/css";
const char *http_content_type_text_js_str = "text/javascript";
const char *http_content_type_application_x_www_form_urlencoded_str = "application/x-www-form-urlencoded";
const char *http_content_type_application_json = "application/json";
const char *http_content_type_image_jpeg = "image/jpeg";
const char *http_content_type_application_octet_stream = "application/octet-stream";

/// Gets the string version of an HTTP content type.
const char *http_content_type_to_string (http_content_type_t type) {
    switch (type) {
    case HTTP_CONTENT_TYPE_TEXT_HTML:
        return http_content_type_text_html_str;
    case HTTP_CONTENT_TYPE_TEXT_PLAIN:
        return http_content_type_text_plain_str;
    case HTTP_CONTENT_TYPE_TEXT_CSS:
        return http_content_type_text_css_str;
    case HTTP_CONTENT_TYPE_TEXT_JS:
        return http_content_type_text_js_str;
    case HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED:
        return http_content_type_application_x_www_form_urlencoded_str;
    case HTTP_CONTENT_TYPE_APPLICATION_JSON:
        return http_content_type_application_json;
    case HTTP_CONTENT_TYPE_IMAGE_JPEG:
        return http_content_type_image_jpeg;
    case HTTP_CONTENT_TYPE_APPLICATION_OCTET_STREAM:
        return http_content_type_application_octet_stream;
    default:
        return NULL;
    }
}

/// Gets an HTTP content type from string.
http_content_type_t http_content_type_from_string (const char *str) {
    if (strcmp (str, http_content_type_text_html_str) == 0)
        return HTTP_CONTENT_TYPE_TEXT_HTML;
    else if (strcmp (str, http_content_type_text_plain_str) == 0)
        return HTTP_CONTENT_TYPE_TEXT_HTML;
    else if (strcmp (str, http_content_type_text_css_str) == 0)
        return HTTP_CONTENT_TYPE_TEXT_CSS;
    else if (strcmp (str, http_content_type_text_js_str) == 0)
        return HTTP_CONTENT_TYPE_TEXT_JS;
    else if (strcmp (str, http_content_type_application_x_www_form_urlencoded_str) == 0)
        return HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED;
    else if (strcmp (str, http_content_type_application_json) == 0)
        return HTTP_CONTENT_TYPE_APPLICATION_JSON;
    else if (strcmp (str, http_content_type_image_jpeg) == 0)
        return HTTP_CONTENT_TYPE_IMAGE_JPEG;
    else if (strcmp (str, http_content_type_application_octet_stream) == 0)
        return HTTP_CONTENT_TYPE_APPLICATION_OCTET_STREAM;
    else
        return HTTP_CONTENT_TYPE_UNKNOWN;
}

/// Gets the HTTP content type from extension.
http_content_type_t http_content_type_from_ext (const char *ext) {
    if (ext == NULL)
        return HTTP_CONTENT_TYPE_UNKNOWN;
    else if (strcmp (ext, "jpg") == 0 || strcmp (ext, "jpeg"))
        return HTTP_CONTENT_TYPE_IMAGE_JPEG;
    else
        return HTTP_CONTENT_TYPE_UNKNOWN;
}
