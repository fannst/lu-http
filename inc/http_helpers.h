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
