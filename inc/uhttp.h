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

#ifdef _WIN32
#include "winsock2.h"

#ifdef _UHTTP_INTERNAL_
#define UHTTPAPI __declspec(dllexport) extern
#else
#define UHTTPAPI __declspec(dllimport) extern
#endif

#else
#define UHTTPAPI extern
#endif

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
    struct 
    {
        struct sockaddr addr;
        int len;
    } bind_addr;
    /* Error callback. */
    uhttp_error_func_t error_func;
} uhttp_option_arg_t;

/**
 * Create uHTTP server object.
 * @return Handle to server object or NULL for failure (see errno)
 */
UHTTPAPI uhttp_server_t* uhttp_create();
/**
 * Destroy a uHTTP server object.
 * @param sv Server object.
 */
UHTTPAPI void uhttp_destroy(uhttp_server_t* sv);

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
UHTTPAPI int uhttp_setoption(uhttp_server_t* sv, uhttp_option_name_t name, const uhttp_option_arg_t *value);

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
UHTTPAPI int uhttp_getoption(uhttp_server_t* sv, uhttp_option_name_t name, uhttp_option_arg_t *value);

/**
 * Start server.
 * @param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTPAPI int uhttp_start(uhttp_server_t* sv);

/**
 * Poll for server events.
 * @param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTPAPI int uhttp_poll(uhttp_server_t* sv);

/**
 * Stop server, close all connections.
 * param sv Server object.
 * @return Zero when successful, see errno otherwise.
 */
UHTTPAPI int uhttp_stop(uhttp_server_t* sv);

#endif
