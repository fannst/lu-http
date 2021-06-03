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

/// Creates the thread pools for the HTTP server socket instance.
int32_t __http_server_socket_new__create_thread_pools (http_server_socket_t *sock) {
    sock->pools = (http_server_socket_thread_pool_t *) calloc (sock->thread_pool_count, sizeof (http_server_socket_thread_pool_t));
    if (sock->pools == NULL) {
        free (sock);
        return -1;
    }

    return 0;
}

/// Creates an new HTTP server socket instance.
int32_t http_server_socket_new (http_server_socket_t **sock, size_t thread_pool_count) {
    // Allocates the memory required for the server socket structure.
    *sock = (http_server_socket_t *) calloc (1, sizeof (http_server_socket_t));
    if (sock == NULL)
        return -1;

    // Sets the default values.
    (*sock)->flags = 0;
    (*sock)->thread_pool_count = thread_pool_count;

    // Creates the memory required for the thread pools.
    if (__http_server_socket_new__create_thread_pools (*sock) < 0) {
        free (*sock);
        *sock = NULL;
        return -2;
    }

    // Creates the socket.
    if (((*sock)->fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror ("socket (AF_INET, SOCK_STREAM, IPPROTO_TCP) failed");

        free ((*sock)->pools);

        free (*sock);
        *sock = NULL;

        return -3;
    }

    // Initializes the mutexes.
    for (size_t i = 0; i < (*sock)->thread_pool_count; ++i) {
        http_server_socket_thread_pool_t *pool = &((*sock)->pools[i]);
        if (pthread_mutex_init (&pool->mutex, NULL) != 0) {
            perror ("pthread_mutex_init () failde");
            return -5;
        }
    }

    if (pthread_mutex_init (&(*sock)->struct_mutex, NULL) != 0) {
        perror ("pthread_mutex_init () failed");
        return -4;
    }

    return 0;
}

/// Frees an HTTP server socket instance.
int32_t http_server_socket_free (http_server_socket_t **sock) {
    // Makes sure that we're not dealing with a null pointer-pointer.
    if (*sock == NULL)
        return -1;

    // Sets the shutdown flag, and waits the thread to be shut down.
    (*sock)->flags |= HTTP_SERVER_SOCKET_THREADS_SHUTDOWN_FLAG;
    
    pthread_join ((*sock)->acceptor_thread_id, NULL);
    for (size_t i = 0; i < (*sock)->thread_pool_count; ++i) {
        http_server_socket_thread_pool_t *pool = &((*sock)->pools[i]);

        pthread_join (pool->thread, NULL);

        __http_socket_pool__close_and_free_all_sockets (pool);
        
        pthread_mutex_destroy (&pool->mutex);
    }

    pthread_mutex_destroy (&(*sock)->struct_mutex);

    // Closes the open socket.
    if (close ((*sock)->fd) < 0) {
        perror ("close () failed");
        return -2;
    }

    // Frees the struture, and sets it to null.
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

///////////////////////////////////////////////////////////////////////////////
// HTTP Server Socket Pooling
///////////////////////////////////////////////////////////////////////////////

/// Closes and removes all sockets.
void __http_socket_pool__close_and_free_all_sockets (http_server_socket_thread_pool_t *pool) {
    pthread_mutex_lock (&pool->mutex);
    
    http_socket_t *current = pool->start;
    while (current != NULL) {
        close (current->fd);
        free (current);

        current = current->next;
    }

    pool->start = pool->end = NULL;
    pool->socket_count = 0;

    pthread_mutex_unlock (&pool->mutex);
}

/// Gets called when an socket can be written to.
int32_t __http_socket_pool__on_writable (http_server_socket_t *sock, http_server_socket_thread_pool_t *pool, http_socket_t *socket) {
    return 0;
}

/// Gets called when an socket can be read from.
int32_t __http_socket_pool__on_readable (http_server_socket_t *sock, http_server_socket_thread_pool_t *pool, http_socket_t *socket) {
    char buffer [1024];

    int rc = recv (socket->fd, buffer, sizeof (buffer), 1024);
    switch (rc) {
    case 0:
        return -1;
    case -1:
        perror ("recv () failed");
        return -2;
    default:
        break;
    }

    return 0;
}

/// Registers an socket to specified pool, only can be called if pool empty.
void __http_socket_pool_register_socket__empty_pool (http_server_socket_thread_pool_t *pool, http_socket_t *socket) {
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
void __http_socket_pool_register_socket__not_empty_pool (http_server_socket_thread_pool_t *pool, http_socket_t *socket) {
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
void __http_socket_pool_register_socket (http_server_socket_thread_pool_t *pool, http_socket_t *socket) {
    pthread_mutex_lock (&pool->mutex);

    // Checks which insertion method we should use.
    if (pool->socket_count == 0)
        __http_socket_pool_register_socket__empty_pool (pool, socket);
    else
        __http_socket_pool_register_socket__not_empty_pool (pool, socket);

    pthread_mutex_unlock (&pool->mutex);
}

/// Unregisters an socket with the specified fd.
void __http_socket_pool_unregister__by_fd (http_server_socket_thread_pool_t *pool, int32_t fd) {
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
        
    free (socket);
}


/// Gets an socket by fd.
http_socket_t *__http_socket_pool__get_socket_by_fd (http_server_socket_thread_pool_t *pool, int32_t fd) {
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
        poll_rc = poll (poll_fds, args->pool->socket_count, 10 * 1000);
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
        if (args->sock->flags & HTTP_SERVER_SOCKET_THREADS_SHUTDOWN_FLAG) {
            break;
        }
    }

    // Frees the thread pool argument, and returns null.
    free (arg);
    return NULL;
}

/// Starts the thread pools.
int32_t http_server_start_thread_pools (http_server_socket_t *sock) {
    // Loops over all the pools, and starts them.
    for (size_t i = 0; i < sock->thread_pool_count; ++i) {
        http_server_socket_thread_pool_t *pool = &sock->pools[i];

        // Creates the thread, and if this goes wrong, we will print the error message
        //  and return error code.
        __http_socket_pool_method__arg *args = (__http_socket_pool_method__arg *) malloc (sizeof (__http_socket_pool_method__arg));
        args->sock = sock;
        args->pool = pool;
        if (pthread_create (&pool->thread, NULL, __http_socket_pool_method, args) < 0) {
            perror ("pthread_create () failed");
            return -1;
        }

        // Prints that we've created the socket pool.
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

    // Configures the http socket.
    socket->address = client_addr;
    socket->fd = fd;
    socket->last_active = time (NULL);

    // Returns the socket.
    return socket;
}

/// Registers an accepted socket
void __http_server_acceptor__register_socket (http_server_socket_t *sock, http_socket_t *socket) {
    pthread_mutex_lock (&sock->struct_mutex);
    sock->thread_pool_register_next = (sock->thread_pool_register_next + 1) % (sock->thread_pool_count);
    http_server_socket_thread_pool_t *pool = &(sock->pools[sock->thread_pool_register_next]);
    pthread_mutex_unlock (&sock->struct_mutex);

    __http_server_socket_log (sock, "Registering socket to pool %lu", sock->thread_pool_register_next);
    __http_socket_pool_register_socket (pool, socket);
}

/// Accepts incomming connections.
void *__http_server_acceptor (void *arg) {
    http_server_socket_t *sock = (http_server_socket_t *) arg;

    // Stays in loop as long as shutdown is not rqeuested.
    for (;;) {
        http_socket_t *socket = NULL;
        if ((socket = __http_server__accept_socket (sock)) != NULL) {
            char *addr = __http_helper__thread_safe__inet_ntoa (socket->address.sin_addr);
            __http_server_socket_log (sock, "Accepted %s:%u\r\n", addr, ntohs (socket->address.sin_port));
            free (addr);

            __http_server_acceptor__register_socket (sock, socket);
        }

        // Checks if we need to shut down acceptor thread.
        if (sock->flags & HTTP_SERVER_SOCKET_THREADS_SHUTDOWN_FLAG) {
            break;
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
    if (pthread_create (&sock->acceptor_thread_id, NULL, __http_server_acceptor, (void *) sock) < 0) {
        perror ("pthread_create () failed");
        return -2;
    }

    // Prints that the acceptor has started.
    __http_server_socket_log (sock, "Acceptor started.");

    return 0;
}
