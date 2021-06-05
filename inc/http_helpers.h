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

#ifndef _HTTP_HELPERS_H
#define _HTTP_HELPERS_H

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

/// Thread safe call to inet_ntoa.
char * __http_helper__thread_safe__inet_ntoa (struct in_addr in);

/// Initializes all the mutexes for http helper functions
void __http_helpers_init__mutex (void);

/// Initializes all the helper functions (if required).
void http_helpers_init (void);

#endif
