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

#include "http_socket.h"

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket
///////////////////////////////////////////////////////////////////////////////

/// Logging method for the socket.
void __http_server_socket_log (http_server_socket_t *sock, const char *format, ...) {
    char *addr = __http_helper__thread_safe__inet_ntoa (sock->address.sin_addr);
    printf ("%lu HTTP Server [%s:%u] -> ", time (NULL), addr, ntohs (sock->address.sin_port));
    free (addr);
    
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    va_end (args);

    printf ("\r\n");
}

/// Creates an new HTTP server socket instance.
http_server_socket_t *http_server_socket_create (size_t thread_pool_count, size_t max_socket_count) {
    // Allocates the memory required for the server socket structure.
    http_server_socket_t *server_socket = (http_server_socket_t *) calloc (1, sizeof (http_server_socket_t));
    if (server_socket == NULL)
        return NULL;

    // Sets the default values.
    server_socket->flags = 0;
    server_socket->thread_pool_count = thread_pool_count;

    // Allocates the memory for the socket pool-pointer array.
    server_socket->pools = (http_server_socket_pool_t **) malloc (thread_pool_count * sizeof (http_server_socket_pool_t *));
    if (server_socket->pools == NULL) {
        free (server_socket);
        return NULL;
    }

    // Creates the socket pool instances.
    for (size_t i = 0; i < server_socket->thread_pool_count; ++i) {
        server_socket->pools[i] = __http_server_socket_pool_create (max_socket_count);
        if (server_socket->pools[i] == NULL) {
            free (server_socket);
            for (size_t j = 0; j < i; ++j)
                __http_server_socket_pool_free (&server_socket->pools[j]);
            return NULL;
        }
    }

    return server_socket;
}

/// Initializes an HTTP server socket instance.
int32_t http_server_socket_init (http_server_socket_t *sock) {
    if ((sock->fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror ("socket () failed");
        return -1;
    }

    return 0;
}

/// Frees an HTTP server socket instance.
int32_t http_server_socket_free (http_server_socket_t **sock) {
    // Destroys the acceptor mutex.
    if (pthread_mutex_destroy (&(*sock)->acceptor_mutex) != 0) {
        perror ("pthread_mutex_destroy () failed");
        return -1;
    }

    // Frees all the individual pools.
    for (size_t i = 0; i < (*sock)->thread_pool_count; ++i)
        __http_server_socket_pool_free (&(*sock)->pools[i]);

    // Frees the actual memory.
    free ((*sock)->pools);
    free (*sock);
    *sock = NULL;

    return 0;
}

/// Configures the specified HTTP server socket instance.
int32_t http_server_socket_configure (http_server_socket_t *sock, uint16_t port, const char *hostname, int32_t backlog) {
    // Sets the backlog.
    sock->backlog = backlog;

    // Configures the address.
    sock->address.sin_family = AF_INET;
    sock->address.sin_port = htons (port);
    sock->address.sin_addr.s_addr = inet_addr (hostname);

    // Sets the socket reuse port and reuse address.
    int reuse = 1;
    if (setsockopt (sock->fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof (reuse)) < 0) {
        perror ("setsockopt (SOL_SOCKET, SO_REUSEADDR) failed");
        return -1;
    } else if (setsockopt (sock->fd, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof (reuse)) < 0) {
        perror ("setsockopt (SOL_SOCKET, SO_REUSEPORT) failed");
        return -2;
    }

    return 0;
}

/// Binds the specified HTTP server socket instance.
int32_t http_server_socket_bind (http_server_socket_t *sock) {
    if (bind (sock->fd, (struct sockaddr *) &sock->address, sizeof (struct sockaddr_in)) < 0) {
        perror ("bind () failed");
        return -1;
    }

    return 0;
}

/// Listens the specified HTTP server socket instance.
int32_t http_server_socket_listen (http_server_socket_t *sock) {
    if (listen (sock->fd, sock->backlog) < 0) {
        perror ("listen () failed");
        return -1;
    }

    return 0;
}

/// Stops an HTTP server socket instance.
int32_t http_server_socket_stop (http_server_socket_t *sock) {

    // Closes the FD.
    if (close (sock->fd) != 0) {
        perror ("close () failed");
        return -2;
    }

    // Closes the acceptor.
    __http_server_socket_stop_acceptor (sock);

    // Closes all the socket pools.
    for (size_t i = 0; i < sock->thread_pool_count; ++i) {
        if (__http_server_socket_pool_stop (sock->pools[i]) != 0) {
            fprintf (stderr, "__http_server_socket_pool_stop () failed");
            return -1;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket Pooling
///////////////////////////////////////////////////////////////////////////////

/// Creates new HTTP server socket pool.
http_server_socket_pool_t *__http_server_socket_pool_create (size_t max_socket_count) {    
    // Allocates the memory for the pool base structure.
    http_server_socket_pool_t *pool = (http_server_socket_pool_t *) calloc (1, sizeof (http_server_socket_pool_t));
    if (pool == NULL)
        return NULL;

    // Allocates the memory for the poll fds.
    struct pollfd *fds = (struct pollfd *) calloc (max_socket_count, sizeof (struct pollfd));
    if (fds == NULL) {
        free (pool);
        return NULL;
    }

    // Sets the pool variables.
    pool->fds = fds;
    pool->max_socket_count = max_socket_count;

    return pool;
}

/// Initializes HTTP server socket pool.
int32_t __http_server_socket_pool_init (http_server_socket_pool_t *pool) {
    // Initializes the mutex.
    if (pthread_mutex_init (&pool->mutex, NULL) != 0) {
        perror ("pthread_mutex_init () failed");
        return -1;
    }

    return 0;
}

/// Starts HTTP server socket pool.
int32_t __http_server_socket_pool_start (http_server_socket_t *sock, http_server_socket_pool_t *pool) {
    // Creates the thead argument, containing socket and pool.
    __http_socket_pool_method__arg *arg = (__http_socket_pool_method__arg *) malloc (sizeof (__http_socket_pool_method__arg));
    if (arg == NULL)
        return -1;
    
    arg->pool = pool;
    arg->sock = sock;
    
    // Starts the threada.
    if (pthread_create (&pool->thread, NULL, __http_socket_pool_method, (void *) arg) != 0) {
        perror ("pthread_create () failed");
        return -2;
    }

    return 0;
}

/// Stops HTTP server socket pool.
int32_t __http_server_socket_pool_stop (http_server_socket_pool_t *pool) {
    // Sets the socket pool shutdown flag.
    pthread_mutex_lock (&pool->mutex);
    pool->flags |= HTTP_SERVER_SOCKET_POOL_FLAG_SHUTDOWN;
    pthread_mutex_unlock (&pool->mutex);

    // Joins the socket pool with the current thread, waiting for it to shutdown.
    if (pthread_join (pool->thread, NULL) != 0) {
        perror ("pthread_join () failed");
        return -1;
    }
    
    // Loops over all the sockets, closes the connections and removes them from
    //  the list.
    http_socket_t *socket = pool->start;
    while (socket != NULL) {
        // Closes the socket, and unregisters it from the list.
        close (socket->fd);

        // Gets the next socket, unregisters the file descriptor
        //  and goes to the next one.
        http_socket_t *next = socket->next; // May be NULL!
        __http_socket_pool_unregister__by_fd (pool, socket->fd);
        socket = next;
    }

    return 0;
}

/// Frees HTTP server socket pool.
void __http_server_socket_pool_free (http_server_socket_pool_t **pool) {
    // Frees the fds struct array.
    free ((*pool)->fds);

    // Frees the structure, and sets it to zero.
    free (*pool);
    *pool = NULL;
}

///////////////////////////////////////////////////////////////////////////////

/// Gets called when an socket can be written to.
int32_t __http_socket_pool__on_writable (http_server_socket_t *sock, http_server_socket_pool_t *pool, http_socket_t *socket) {
    while (socket->write_line_buffer->line_count > 0) {
        __http_line_buffer__line_t *line = http_line_buffer_get_latest_line (socket->write_line_buffer);

        int32_t rc = write (socket->fd, &line->string[line->written], line->total - line->written);
        printf ("%lu\r\n", line->total);
        if (rc == -1) {
            perror ("write () failed");
            return -1;
        } else if (rc == (int32_t) line->total) {
            http_line_buffer_free_latest_line (socket->write_line_buffer);
        } else {
            line->written = (size_t) rc;
            break;
        }
    }

    return 0;
}

/// Gets called when an socket can be read from.
int32_t __http_socket_pool__on_readable (http_server_socket_t *sock, http_server_socket_pool_t *pool, http_socket_t *socket) {
    int rc = recv (socket->fd, &socket->recv_buffer[socket->recv_buffer_level], HTTP_SOCKET_RECV_BUFFER_SIZE - socket->recv_buffer_level, 0);
    switch (rc) {
    case 0:
        return -1;
    case -1:
        perror ("recv () failed");
        return -2;
    default:
        break;
    }

    socket->recv_buffer_level += (size_t) rc;

    // Reads all the lines from the buffer.
    size_t offset = 0;
    for (;;) {
        char *start = &socket->recv_buffer[offset];
        char *lf = memchr ((const void *) start, '\n', socket->recv_buffer_level - offset);
        if (lf == NULL) {
            break;
        }

        size_t line_len = lf - start;
        if (line_len > 1) {
            lf [lf[-1] == '\r' ? -1 : 0] = '\0';
            http_line_buffer_append (socket->read_line_buffer, http_line_buffer_line_create_from_string (start));
        } else {
            http_line_buffer_append (socket->read_line_buffer, http_line_buffer_line_create_from_string (""));
        }

        offset += line_len + 1;
    }

    // Moves the buffer so we can read more in the future.
    memmove (socket->recv_buffer, &socket->recv_buffer[offset], socket->recv_buffer_level - offset);
    socket->recv_buffer_level -= offset;
    
    http_request_t req;
    req.headers = http_headers_new ();
    req.state = HTTP_REQUEST_STATE_RECEIVING_TYPE;
    while (socket->read_line_buffer->line_count > 0) {
        __http_line_buffer__line_t *line = http_line_buffer_get_latest_line (socket->read_line_buffer);
        http_request_update (&req, (char *) line->string);
        http_line_buffer_free_latest_line (socket->read_line_buffer);
    }

    http_request_print (&req);

    http_headers_free (&req.headers);
    free (req.url);

    return 0;
}

/// Registers an socket to specified pool, only can be called if pool empty.
void __http_socket_pool_register_socket__empty_pool (http_server_socket_pool_t *pool, http_socket_t *socket) {
    // Make the start and end the specified socket.
    pool->start = socket;
    pool->end = socket;

    // Since there are no other elements in the doubly-linked list, set next and prev to NULL.
    socket->next = NULL;
    socket->prev = NULL;

    // Increments the number of sockets in pool.
    ++pool->socket_count;
}

/// Registers an socket to the specified pool, only can be called if pool not empty.
void __http_socket_pool_register_socket__not_empty_pool (http_server_socket_pool_t *pool, http_socket_t *socket) {
    // Sets the previous value of the new socket to the current end.
    socket->prev = pool->end;
    socket->prev->next = socket;
    socket->next = NULL;

    // Make the new socket the last element in the list
    pool->end = socket;

    // Increment the number of sockets in pool.
    ++pool->socket_count;
}

/// Registers an socket to the specified pool
void __http_socket_pool_register_socket (http_server_socket_pool_t *pool, http_socket_t *socket) {
    if (pool->socket_count == 0)
        __http_socket_pool_register_socket__empty_pool (pool, socket);
    else
        __http_socket_pool_register_socket__not_empty_pool (pool, socket);
}

/// Unregisters an socket with the specified fd.
void __http_socket_pool_unregister__by_fd (http_server_socket_pool_t *pool, int32_t fd) {
    http_socket_t *socket = __http_socket_pool__get_socket_by_fd (pool, fd);
    if (socket == NULL) {
        fprintf (stderr, "Attempting to unregister socket of value NULL\r\n");
        return;
    }

    if (socket->next != NULL)
        socket->next->prev = socket->prev;

    if (socket->prev != NULL)
        socket->prev->next = socket->next;

    if (pool->start == socket)
        pool->start = socket->next;

    if (pool->end == socket)
        pool->end = socket->prev;

    --pool->socket_count;
    
    http_line_buffer_free (&socket->read_line_buffer);
    http_line_buffer_free (&socket->write_line_buffer);

    free (socket->recv_buffer);
    free (socket);
}


/// Gets an socket by fd.
http_socket_t *__http_socket_pool__get_socket_by_fd (http_server_socket_pool_t *pool, int32_t fd) {
    http_socket_t *res = NULL;

    http_socket_t *current = pool->start;
    while (current != NULL) {
        if (current->fd == fd) {
            res = current;
            break;
        }

        current = current->next;
    }

    return res;
}

/// Event loop for HTTP server pool process.
void *__http_socket_pool_method (void *arg) {
    struct pollfd poll_fds[1024];
    __http_socket_pool_method__arg *args = (__http_socket_pool_method__arg *) arg;

    // Stays in loop as long as shutdown is not rqeuested.
    for (;;) {
        // Locks the mutex and allocates the memory which we will use to store
        //  the polling results in.
        
        pthread_mutex_lock (&args->pool->mutex);
        
        // Loops over all the sockets specified in the pool information, after which
        //  we construct our list of poll file descriptors and register each one of them.
        
        {
            const http_socket_t *socket = args->pool->start;
            for (size_t i = 0; i < args->pool->socket_count; ++i) {
                if (socket == NULL) {
                    fprintf (stderr, "socket->next == NULL, this should not happen normally!\r\n");
                }
                
                poll_fds[i].events = POLLIN | POLLOUT | POLLERR | POLLHUP;
                poll_fds[i].fd = socket->fd;
                
                socket = socket->next;
            }
        }

        // Unlocks the mutex (allows new clients to be added)
        pthread_mutex_unlock (&args->pool->mutex);

        // Polls the FD's with an timeout of 1 millisecond, after which we check the return
        //  code and (possibly) jump to retry, else we either print an error or continue further.

        int poll_rc;

    poll_retry:
        poll_rc = poll (poll_fds, args->pool->socket_count, 50);
        if (poll_rc == -1) {
            // Checks if the errno tells us to try again.
            if (errno == EAGAIN)
                goto poll_retry;

            // Since it's an actual error message, print it.
            perror ("poll () failed");
            continue;
        }

        // Loops over all the sockets in the pool and polls for events, if one of them
        //  fail we will mark the socket as 'to close' and later remove it from the linked
        //  list.

        for (int32_t i = 0; i < poll_rc; ++i) {
            int16_t revents = poll_fds[i].revents;
            bool should_close = false;

            pthread_mutex_lock (&args->pool->mutex);
            http_socket_t *socket = __http_socket_pool__get_socket_by_fd (args->pool, poll_fds[i].fd);
            pthread_mutex_unlock (&args->pool->mutex);

            if (revents & POLLIN) {
                if (__http_socket_pool__on_readable (args->sock, args->pool, socket) != 0) {
                    should_close = true;
                }
            }

            if (revents & POLLOUT) {
                if (__http_socket_pool__on_writable (args->sock, args->pool, socket) != 0) {
                    should_close = true;
                }
            }

            // If an error has occured, or the connection has been closed, lock the mutex
            //  and mark the socket as 'should close'.

            if (revents & POLLERR || revents & POLLHUP || should_close) {
                pthread_mutex_lock (&args->pool->mutex);
                
                close (socket->fd);
                __http_server_socket_log (args->sock, "Connection closed to socket %d", socket->fd);
                __http_socket_pool_unregister__by_fd (args->pool, socket->fd);

                pthread_mutex_unlock (&args->pool->mutex);
            }
        }

        // Checks if we need to shut down acceptor thread.
        if (args->pool->flags & HTTP_SERVER_SOCKET_POOL_FLAG_SHUTDOWN) {
            __http_server_socket_log (args->sock, "Pool received shutdown signal ...");
            break;
        }
    }

    // Frees the thread pool argument, and returns null.
    free (arg);
    return NULL;
}

/// Starts the thread pools.
int32_t http_server_start_thread_pools (http_server_socket_t *sock) {
    for (size_t i = 0; i < sock->thread_pool_count; ++i) {
        __http_server_socket_pool_start (sock, sock->pools[i]);
        __http_server_socket_log (sock, "Socket pool %lu of %lu created and running.", i, sock->thread_pool_count);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket Acceptor
///////////////////////////////////////////////////////////////////////////////


/// Accepts an client socket, returns NULL if not possible.
http_socket_t *__http_server__accept_socket (http_server_socket_t *sock) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof (client_addr);

    // Accepts the new client socket, and if this returns < 0 we will return
    //  NULL since nothing got accepted.
    int32_t fd = accept (sock->fd, (struct sockaddr *) &client_addr, &client_addr_len);
    if (fd < 0) {
        return NULL;
    }

    // Makes the socket non-blocking, this is important since we need to
    //  use polling.
    int flags;
    if ((flags = fcntl (fd, F_GETFL)) < 0) {
        perror ("fcntl (F_GETFL) failed");
        return NULL;
    } else if (fcntl (fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror ("fcntl (F_SETFL) failed");
        return NULL;
    }

    // Allocates the memory required to keep the HTTP socket, if this fails
    //  close the FD and return null.
    http_socket_t *socket = (http_socket_t *) calloc (1, sizeof (http_socket_t));
    if (socket == NULL) {
        close (fd);
        return NULL;
    }

    // Create the read line buffer.
    socket->read_line_buffer = http_line_buffer_create ();
    if (socket->read_line_buffer == NULL) {
        close (fd);
        free (socket);
        return NULL;
    }

    // Create the write line buffer.
    socket->write_line_buffer = http_line_buffer_create ();
    if (socket->write_line_buffer == NULL) {
        close (fd);

        http_line_buffer_free (&socket->read_line_buffer);

        free (socket);
        return NULL;
    }

    // Creates the line buffer.
    socket->recv_buffer = (char *) malloc (HTTP_SOCKET_RECV_BUFFER_SIZE);
    if (socket->recv_buffer == NULL) {
        close (fd);

        http_line_buffer_free (&socket->read_line_buffer);
        http_line_buffer_free (&socket->write_line_buffer);

        free (socket);
        return NULL;
    }

    // Configures the http socket.
    socket->address = client_addr;
    socket->fd = fd;
    socket->last_active = time (NULL);

    // Returns the socket.
    return socket;
}

/// Registers an accepted socket
void __http_server_acceptor__register_socket (http_server_socket_t *sock, http_socket_t *socket) {
    // Locks the mutex.
    pthread_mutex_lock (&sock->acceptor_mutex);

    // Determines which pool to register the socket to, after which we get it and register
    //  the scket to it.
    sock->thread_pool_register_next = (sock->thread_pool_register_next + 1) % (sock->thread_pool_count);
    http_server_socket_pool_t *pool = sock->pools[sock->thread_pool_register_next];
    __http_socket_pool_register_socket (pool, socket);

    // Unlocks the mutex and logs that we've registered the socket.
    pthread_mutex_unlock (&sock->acceptor_mutex);
    __http_server_socket_log (sock, "Registering socket to pool %lu", sock->thread_pool_register_next);

}

/// Accepts incomming connections.
void *__http_server_acceptor (void *arg) {
    http_server_socket_t *sock = (http_server_socket_t *) arg;

    // Stays in loop as long as shutdown is not rqeuested.
    for (;;) {
        http_socket_t *socket = NULL;
        if ((socket = __http_server__accept_socket (sock)) != NULL) {
            char *addr = __http_helper__thread_safe__inet_ntoa (socket->address.sin_addr);
            __http_server_socket_log (sock, "Accepted %s:%u", addr, ntohs (socket->address.sin_port));
            free (addr);

            __http_server_acceptor__register_socket (sock, socket);
        }
    }

    return NULL;
}

/// Starts the accepting thread.
int32_t http_server_start_acceptor (http_server_socket_t *sock, bool in_new_thread) {
    // Makes sure the acceptor thread is not running already.
    if (sock->flags & HTTP_SERVER_SOCKET_ACCEPTOR_THREAD_CREATED) {
        fprintf (stderr, "Acceptor thread already running.\r\n");
        return -1;
    }

    // Sets the acceptor thread created flag, to prevent it from being created another time.
    sock->flags |= HTTP_SERVER_SOCKET_ACCEPTOR_THREAD_CREATED;

    // Checks if we want the acceptor in the current thread, if so just call the
    //  acceptor function.
    if (!in_new_thread) {
        __http_server_acceptor (sock);
        return 0;
    }

    // Creates the new acceptor thread, and detaches it from the current thread.
    if (pthread_create (&sock->acceptor_thread, NULL, __http_server_acceptor, (void *) sock) < 0) {
        perror ("pthread_create () failed");
        return -2;
    }

    // Prints that the acceptor has started.
    __http_server_socket_log (sock, "Acceptor started.");

    return 0;
}

/// Stops the acceptor.
int32_t __http_server_socket_stop_acceptor (http_server_socket_t *sock) {
    // Joints the thread, so we basically wait for it to quit.
    if (pthread_cancel (sock->acceptor_thread) != 0) {
        perror ("pthread_join () failed");
        return -1;
    }


    return 0;
}