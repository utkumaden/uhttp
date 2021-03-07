/**
 * Copyright 2021 H. Utku Maden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#ifndef _UHTTP_H_
#define _UHTTP_H_

#include <stddef.h>
#include <stdint.h>

#if _WIN32
#include "winsock2.h"

#ifdef _UHTTP_INTERNAL_
#define UHTTP_EXTERN __declspec(dllexport) extern
#else
#define UHTTP_EXTERN __declspec(dllimport) extern
#endif

typedef SOCKET uhttp_socket_t;
typedef int64_t ssize_t;

#else
#define UHTTP_EXTERN extern

typedef int uhttp_socket_t;
#endif

/* UHTTP SOCKETS */

typedef enum uhttp_socket_domain_t {
    UHTTP_SOCKET_DOMAIN_INET4 = 4,
    UHTTP_SOCKET_DOMAIN_INET6 = 6
} uhttp_socket_domain_t;

typedef enum uhttp_event_t {
    UHTTP_EVENT_HANGUP  = 1,
    UHTTP_EVENT_ERROR   = 2,
    UHTTP_EVENT_RECEIVE = 4
} uhttp_event_t;

typedef struct uhttp_addr_t
{
    uhttp_socket_domain_t      domain;
    uint16_t                   port;
    uint8_t                    address[16];
} uhttp_addr_t;

#define UHTTP_INVALID_SOCKET ((uhttp_socket_t)-1)


/**
 * Initalize socket subsystem.
 * @return Zero when successful, else see errno.
 */
UHTTP_EXTERN int uhttp_socket_init();

/**
 * Clean up socket susbsystem.
 */
UHTTP_EXTERN void uhttp_socket_deinit();

/**
 * Create a socket.
 * @param Binding address of the socket.
 * @return A valid socket object or UHTTP_INVALID_SOCKET.
 */
UHTTP_EXTERN uhttp_socket_t uhttp_socket(uhttp_addr_t* addr);

/**
 * Listen socket.
 * @param sock Socket object.
 * @param backlog Length of connection queue.
 * @return Zero when successfull, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_listen(uhttp_socket_t sock, int backlog);

/**
 * Set or reset socket async mode.
 * @param sock Socket object.
 * @param flag Zero to reset, anything else to reset.
 * @return Zero when successful, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_async(uhttp_socket_t sock, int flag);

/**
 * Accept connection.
 * @param sock Socket object.
 * @param addr Source address of new socket object.
 * @return A new socket object or UHTTP_INVALID_SOCKET if no incoming connections or an error.
 */
UHTTP_EXTERN uhttp_socket_t uhttp_accept(uhttp_socket_t sock, uhttp_addr_t *addr);

/**
 * Poll socket events.
 * @param socks Socket to poll.
 * @param events Bitmap of events.
 * @return Zero if successfull, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_poll(uhttp_socket_t sock, uhttp_event_t* events);

/**
 * Recieve data.
 * @param sock Socket object.
 * @param buffer Buffer to read into.
 * @param len Length of buffer.
 * @return Number of bytes read, or -1 for error (see errno).
 */
UHTTP_EXTERN ssize_t uhttp_recv(uhttp_socket_t sock, void* buffer, size_t len);

/**
 * Send data.
 * @param sock Socket object.
 * @param buffer Buffer to write from.
 * @param len Length of data to send.
 * @return Number of bytes sent, or -1 for error (see errno).
 */
UHTTP_EXTERN ssize_t uhttp_send(uhttp_socket_t sock, const void* buffer, size_t len);

/**
 * Close socket.
 * @param sock Socket object.
 */
UHTTP_EXTERN void uhttp_close(uhttp_socket_t sock);

/* UHTTP SERVER */

/**
 * uHTTP server object.
 */
typedef struct uhttp_server_t uhttp_server_t;

/**
 * uHTTP error callback function pointer.
 */
typedef void (*uhttp_error_func_t)(int number, const char* description);

/**
 * Server option names.
 * @see uhttp_setoption
 * @see uhttp_getoption
 */
typedef enum uhttp_option_name_t {
    UHTTP_OPTION_BIND_ADDR = 1,
    UHTTP_OPTION_BACKLOG = 2,
    UHTTP_OPTION_ERROR_FUNC = 3
} uhttp_option_name_t;

/**
 * Server option arguments.
 * @see uhttp_setoption
 * @see uhttp_getoption
 */
typedef union uhttp_option_arg_t {
    /* Any option with a singular integer argument.*/
    int integer;
    /* Binding address. */
    uhttp_addr_t addr;
    /* Error callback. */
    uhttp_error_func_t error_func;
} uhttp_option_arg_t;

/**
 * Create uHTTP server object.
 * @return Handle to server object or NULL for failure (see errno)
 */
UHTTP_EXTERN uhttp_server_t* uhttp_create();
/**
 * Destroy a uHTTP server object.
 * @param sv Server object.
 */
UHTTP_EXTERN void uhttp_destroy(uhttp_server_t* sv);

/**
 * Set server options.
 * @param sv Server object.
 * @param name Option name.
 * @param value Pointer to option value.
 * @return Zero when successful, see errno otherwise.
 * 
 * @see uhttp_option_name_t
 * @see uhttp_option_arg_t
 */
UHTTP_EXTERN int uhttp_setoption(uhttp_server_t* sv, uhttp_option_name_t name, const uhttp_option_arg_t *value);

/**
 * Get server options.
 * @param sv Server object.
 * @param name Option name.
 * @param value Pointer to option value.
 * @return Zero when successful, see errno otherwise.
 * 
 * @see uhttp_option_name_t
 * @see uhttp_option_arg_t
 */
UHTTP_EXTERN int uhttp_getoption(uhttp_server_t* sv, uhttp_option_name_t name, uhttp_option_arg_t *value);

/**
 * Start server.
 * @param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_start(uhttp_server_t* sv);

/**
 * Poll for server events.
 * @param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_pollevents(uhttp_server_t* sv);

/**
 * Stop server, close all connections.
 * param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTP_EXTERN int uhttp_stop(uhttp_server_t* sv);

#endif
