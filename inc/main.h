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

#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <argp.h>
#include <signal.h>

#include "http_socket.h"
#include "http_header.h"
#include "http_response.h"
#include "http_helpers.h"
#include "http_url.h"
#include "router/http_router.h"

int main (int argc, char **argv);

#endif
