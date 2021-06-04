#ifndef _HTTP_SOCKET_H
#define _HTTP_SOCKET_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "http_helpers.h"

///////////////////////////////////////////////////////////////////////////////
// Flags and shit
///////////////////////////////////////////////////////////////////////////////

#define HTTP_SERVER_SOCKET_ACCEPTOR_THREAD_CREATED          (1 << 1)

#define HTTP_SERVER_SOCKET_POOL_FLAG_SHUTDOWN               (1 << 0)

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////

struct http_socket {
    struct sockaddr_in address;
    int32_t fd;
    int64_t last_active;
    uint32_t flags;

    struct http_socket *next, *prev;
};

typedef struct http_socket http_socket_t;

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;

    uint32_t socket_count;
    http_socket_t *start, *end;

    uint32_t flags;

    size_t max_socket_count;
    struct pollfd *fds;
} http_server_socket_pool_t;

typedef struct {
    struct sockaddr_in address;
    int32_t fd, backlog;
    uint32_t flags;

    http_server_socket_pool_t **pools;
    size_t thread_pool_count;

    pthread_mutex_t acceptor_mutex;
    pthread_t acceptor_thread;

    size_t thread_pool_register_next;
} http_server_socket_t;

typedef struct {
    http_server_socket_t *sock;
    http_server_socket_pool_t *pool;
} __http_socket_pool_method__arg;

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket
///////////////////////////////////////////////////////////////////////////////

/// Logging method for the socket.
void __http_server_socket_log (http_server_socket_t *sock, const char *format, ...);

/// Creates an new HTTP server socket instance.
http_server_socket_t *http_server_socket_create (size_t thread_pool_count, size_t max_socket_count);

/// Initializes an HTTP server socket instance.
int32_t http_server_socket_init (http_server_socket_t *sock);

/// Frees an HTTP server socket instance.
int32_t http_server_socket_free (http_server_socket_t **sock);

/// Configures the specified HTTP server socket instance.
int32_t http_server_socket_configure (http_server_socket_t *sock, uint16_t port, const char *hostname, int32_t backlog);

/// Binds the specified HTTP server socket instance.
int32_t http_server_socket_bind (http_server_socket_t *sock);

/// Listens the specified HTTP server socket instance.
int32_t http_server_socket_listen (http_server_socket_t *sock);

/// Stops an HTTP server socket instance.
int32_t http_server_socket_stop (http_server_socket_t *sock);

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket Pooling
///////////////////////////////////////////////////////////////////////////////

/// Creates new HTTP server socket pool.
http_server_socket_pool_t *__http_server_socket_pool_create (size_t max_socket_count);

/// Initializes HTTP server socket pool.
int32_t __http_server_socket_pool_init (http_server_socket_pool_t *pool);

/// Starts HTTP server socket pool.
int32_t __http_server_socket_pool_start (http_server_socket_t *sock, http_server_socket_pool_t *pool);

/// Stops HTTP server socket pool.
int32_t __http_server_socket_pool_stop (http_server_socket_pool_t *pool);

/// Frees HTTP server socket pool.
void __http_server_socket_pool_free (http_server_socket_pool_t **pool);

///////////////////////////////////////////////////////////////////////////////

/// Gets called when an socket can be written to.
int32_t __http_socket_pool__on_writable (http_server_socket_t *sock, http_server_socket_pool_t *pool, http_socket_t *socket);

/// Gets called when an socket can be read from.
int32_t __http_socket_pool__on_readable (http_server_socket_t *sock, http_server_socket_pool_t *pool, http_socket_t *socket);

/// Registers an socket to specified pool, only can be called if pool empty.
void __http_socket_pool_register_socket__empty_pool (http_server_socket_pool_t *pool, http_socket_t *socket);

/// Registers an socket to the specified pool, only can be called if pool not empty.
void __http_socket_pool_register_socket__not_empty_pool (http_server_socket_pool_t *pool, http_socket_t *socket);

/// Gets an socket by fd.
http_socket_t * __http_socket_pool__get_socket_by_fd (http_server_socket_pool_t *pool, int32_t fd);

/// Unregisters an socket with the specified fd.
void __http_socket_pool_unregister__by_fd (http_server_socket_pool_t *pool, int32_t fd);

/// Registers an socket to the specified pool
void __http_socket_pool_register_socket (http_server_socket_pool_t *pool, http_socket_t *socket);

/// Event loop for HTTP server pool process.
void *__http_socket_pool_method (void *arg);

/// Starts the thread pools.
int32_t http_server_start_thread_pools (http_server_socket_t *sock);

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket Acceptor
///////////////////////////////////////////////////////////////////////////////

/// Accepts an client socket, returns NULL if not possible.
http_socket_t *__http_server__accept_socket (http_server_socket_t *sock);

/// Accepts incomming connections.
void *__http_server_acceptor (void *arg);

/// Starts the accepting thread.
int32_t http_server_start_acceptor (http_server_socket_t *sock, bool in_new_thread);

/// Stops the acceptor.
int32_t __http_server_socket_stop_acceptor (http_server_socket_t *sock);

#endif
