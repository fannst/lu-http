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

#ifndef _HTTP_SOCKET_H
#define _HTTP_SOCKET_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdarg.h>
#include <malloc.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <sys/poll.h>

#include "http_response.h"
#include "http_request.h"
#include "http_helpers.h"
#include "http_segmented_buffer.h"

///////////////////////////////////////////////////////////////////////////////
// Flags and shit
///////////////////////////////////////////////////////////////////////////////

#define HTTP_SERVER_SOCKET_ACCEPTOR_THREAD_CREATED          (1 << 1)

#define HTTP_SERVER_SOCKET_POOL_FLAG_SHUTDOWN               (1 << 0)

#define HTTP_SOCKET_RECV_BUFFER_SIZE                        1024

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////


typedef enum {
    HTTP_SOCKET_WRITE_OP_BYTES,                 /* Large Binary Buffer */
    HTTP_SOCKET_WRITE_OP_FILE                   /* Read All From File */
} http_socket_write_op_type_t;

#define HTTP_SOCKET_WRITE_OP_FLAG__CLOSE_SOCK_AFTER     (1 << 0)
#define HTTP_SOCKET_WRITE_OP_FLAG__FREE_BYTES           (1 << 1)
#define HTTP_SOCKET_WRITE_OP_FLAG__CLOSE_FD             (1 << 2)

/// Doing all in one structure to avoid too-small memory allocations, and after all
///  who gives a damn about idk a few bytes? Maybe you.. Fag.
struct http_socket_write_op {
    http_socket_write_op_type_t         op;
    uint32_t                            flags;
    //---------------------------//
    uint8_t                            *bytes;
    size_t                              size;
    size_t                              bytes_written;
    FILE                               *file;
    off_t                               file_offset;
    //---------------------------//
    struct http_socket_write_op        *next;
    struct http_socket_write_op        *prev;
};
typedef struct http_socket_write_op http_socket_write_op_t;

struct http_socket {
    struct sockaddr_in                  address;
    //---------------------------//

    int32_t                             fd;
    int64_t                             creation_time;
    uint32_t                            flags;
    //---------------------------//
    struct http_socket                 *next;
    struct http_socket                 *prev;
    http_socket_write_op_t             *write_start;
    http_socket_write_op_t             *write_end;
    size_t                              n_pending_write_ops;
    //---------------------------//
    size_t                              recv_buffer_level;
    uint8_t                            *recv_buffer;
    //---------------------------//
    http_request_t *request;
};

typedef struct http_socket http_socket_t;

typedef void (*http_server_callback_t) (http_socket_t *, const http_request_t *, http_response_t *);

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

    http_server_callback_t callback;
} http_server_socket_t;

typedef struct {
    http_server_socket_t *sock;
    http_server_socket_pool_t *pool;
} __http_socket_pool_method__arg;

///////////////////////////////////////////////////////////////////////////////
// HTTP Socket Write Operation
///////////////////////////////////////////////////////////////////////////////

/// Creates an file write operation, for the specified file path.
http_socket_write_op_t *http_socket_write_op_create__file (const char *path);

/// Creates an binary write operation where the memory get's either copied, or just referenced.
http_socket_write_op_t *http_socket_write_op_create__binary (uint8_t *data, size_t size, bool should_copy);

/// Creates an write operation.
http_socket_write_op_t *http_socket_write_op_create (http_socket_write_op_type_t type, void *data, uint32_t flags);

/// Frees an write operation.
int32_t http_socket_write_op_free (http_socket_write_op_t **op);

/// Writes an file to the socket.
int32_t __http_socket_write_op_write__file (http_socket_t *socket, http_socket_write_op_t *op);

/// Writes an bytes to the socket.
int32_t __http_socket_write_op_write__bytes (http_socket_t *socket, http_socket_write_op_t *op);

/// Writes the specified operation.
int32_t http_socket_write_op_write (http_socket_t *socket, http_socket_write_op_t *op);

/// Enqueues an write operation when there is nothing else inside.
void __http_socket_enqueue_write_op__single (http_socket_t *socket, http_socket_write_op_t *op);

/// Enqueues an write operation when there is already one inside.
void __http_socket_enqueue_write_op__multiple (http_socket_t *socket, http_socket_write_op_t *op);

/// Enqueues an write operation to http socket.
void http_socket_enqueue_write_op (http_socket_t *socket, http_socket_write_op_t *op);

/// Dequeues the last element from the queue, most likely called when written.
void http_socket_dequeue_write_op (http_socket_t *socket);

///////////////////////////////////////////////////////////////////////////////
// HTTP Socket
///////////////////////////////////////////////////////////////////////////////

/// Creates new HTTP socket instance.
http_socket_t *http_socket_new (void);

/// Frees HTTP socket instance.
int32_t http_socket_free (http_socket_t **socket);

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket
///////////////////////////////////////////////////////////////////////////////

/// Logging method for the socket.
void __http_server_socket_log (http_server_socket_t *sock, const char *format, ...);

/// Creates an new HTTP server socket instance.
http_server_socket_t *http_server_socket_create (size_t thread_pool_count, size_t max_socket_count, http_server_callback_t callback);

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

/// Processes the request body line-wise, this is done for headers and type.
int32_t __http_socket_pool__on_readable__process_lines (http_server_socket_t *sock, http_server_socket_pool_t *pool, http_socket_t *socket);

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
