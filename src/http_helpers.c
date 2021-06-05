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

#include "http_helpers.h"

pthread_mutex_t __inet_ntoa_mutex;

/// Thread safe call to inet_ntoa.
char *__http_helper__thread_safe__inet_ntoa (struct in_addr in) {
    char *res = NULL, *original = NULL;

    // Locks the mutex.
    pthread_mutex_lock (&__inet_ntoa_mutex);

    // Attempts to get the string version of the specified address, we will
    //  only copy the result if it's not null.
    if ((original = inet_ntoa (in)) != NULL) {
        size_t original_size = strlen (original) + 1;

        char *new = (char *) malloc (original_size);
        memcpy (new, original, original_size);

        res = new;
    }

    // Unlocks the mutex.
    pthread_mutex_unlock (&__inet_ntoa_mutex);

    return res;
}

/// Initializes all the mutexes for http helper functions
void __http_helpers_init__mutex (void) {
    if (pthread_mutex_init (&__inet_ntoa_mutex, NULL) != 0) {
        perror ("pthread_mutex_init () failed");
        return;
    }
}

/// Initializes all the helper functions (if required).
void http_helpers_init (void) {
    __http_helpers_init__mutex ();
}
