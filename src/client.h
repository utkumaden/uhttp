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
#ifndef _UHTTP_INTERNAL_
#error "This is a uHTTP internal header, don't include this file."
#endif

#ifndef _UHTTP_INTERNAL_CLIENT_H_
#define _UHTTP_INTERNAL_CLIENT_H_

#include "uhttp.h"

typedef struct uhttp_client_t
{
    uhttp_server_t* sv;

    uhttp_socket_t sck;
    uhttp_event_t events;
    uhttp_addr_t src;

} uhttp_client_t;

/**
 * Create client object.
 * @param client Client object.
 * @return Zero when successfull, see errno otherwise.
 */
extern int uhttp_client_create(uhttp_client_t* client);
/**
 * Destroy client object.
 * @param Client object.
 */
extern void uhttp_client_destroy(uhttp_client_t* client);

/**
 * Invoke server to close client object.
 * @param Client object.
 */
extern void uhttp_server_close_client(uhttp_client_t* client);

/**
 * Do client events.
 * @param Client object.
 */
extern int uhttp_client_event(uhttp_client_t* client);

#endif
