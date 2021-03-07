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
#define _UHTTP_INTERNAL_

#include "uhttp.h"

#include "list.h"
#include "client.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define UHTTP_BACKLOG_DEFAULT 16

struct uhttp_server_t
{
    /* Listen socket */
    SOCKET sck;

    /* Length of socket backlog. */
    int backlog;

    /* Bound socket address. */
    uhttp_addr_t addr;

    /* Clients list. */
    uhttp_list_t clients;

    /* Error function. */
    uhttp_error_func_t on_error;

};

void uhttp_error_default(int number, const char* description)
{
#ifdef _DEBUG
    fprintf(stderr, "uhttp: (errno=%d) %s\n", number, description);
#endif
}

UHTTP_EXTERN uhttp_server_t* uhttp_create()
{
    uhttp_server_t* sv = malloc(sizeof(uhttp_server_t));

    if (sv)
    {
        // Initialize sockets list.
        sv->sck = -1;

        // Set backlog to default value.
        sv->backlog = UHTTP_BACKLOG_DEFAULT;

        // Clear bind address.
        memset(&sv->addr, 0, sizeof(sv->addr));

        // Init client list.
        uhttp_list_create(&sv->clients, sizeof(uhttp_client_t));

        // Set error callback.
        sv->on_error = uhttp_error_default;
    }

    return sv;
}

UHTTP_EXTERN void uhttp_destroy(uhttp_server_t* sv)
{
    if (sv)
    {
        uhttp_stop(sv);
    }

    free(sv);
}

UHTTP_EXTERN int uhttp_setoption(uhttp_server_t* sv, uhttp_option_name_t name, const uhttp_option_arg_t* value)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    switch (name)
    {
    default:
        errno = EINVAL;
        sv->on_error(EINVAL, "Unknown option (uhttp_setoption)");
        return -1;
    case UHTTP_OPTION_BIND_ADDR:
        memcpy(&sv->addr, &value->addr, sizeof(sv->addr));
        return 0;
    case UHTTP_OPTION_BACKLOG:
        sv->backlog = (value->integer) ? value->integer : UHTTP_BACKLOG_DEFAULT;
        return 0;
    case UHTTP_OPTION_ERROR_FUNC:
        if (value->error_func == NULL)
        {
            sv->on_error = uhttp_error_default;
        }
        else
        {
            sv->on_error = value->error_func;
        }
        return 0;
    }
}

UHTTP_EXTERN int uhttp_getoption(uhttp_server_t* sv, uhttp_option_name_t name, uhttp_option_arg_t* value)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    switch (name)
    {
    default:
        errno = EINVAL;
        sv->on_error(EINVAL, "Unknown option (uhttp_getoption)");
        return -1;
    case UHTTP_OPTION_BIND_ADDR:
        memcpy(&value->addr, &sv->addr, sizeof(sv->addr));
        return 0;
    case UHTTP_OPTION_BACKLOG:
        value->integer = sv->backlog;
        return 0;
    case UHTTP_OPTION_ERROR_FUNC:
        if (sv->on_error == uhttp_error_default)
        {
            value->error_func = NULL;
        }
        else
        {
            value->error_func = sv->on_error;
        }
        return 0;
    }
}

UHTTP_EXTERN int uhttp_start(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Allocate listen socket.
    sv->sck = uhttp_socket(&sv->addr);

    if (sv->sck == UHTTP_INVALID_SOCKET)
    {
        return -1;
    }

    // Listen on socket.
    if (uhttp_listen(sv->sck, sv->backlog))
    {
        return -1;
    }

    return uhttp_async(sv->sck, 1);
}

UHTTP_EXTERN int uhttp_pollevents(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Try accepting new sockets.
    {
        uhttp_addr_t addr;
        uhttp_socket_t xsck;
        while ((xsck = uhttp_accept(sv->sck, &addr)) != UHTTP_INVALID_SOCKET)
        {
            uhttp_client_t client;
            if (uhttp_client_create(&client))
            {
                sv->on_error(errno, "Could not create client object. (uhttp_poll)");
                uhttp_close(xsck);
                continue;
            }

            client.sck = xsck;
            client.sv = sv;
            memcpy(&client.src, &addr, sizeof(addr));

            if (uhttp_list_append(&sv->clients, &client))
            {
                sv->on_error(errno, "Could not append client to list.");
                uhttp_client_destroy(&client);
                continue;
            }
        }
    }

    // Poll all open sockets.
    {
        for (size_t i = 0; i < sv->clients.nlen; i++)
        {
            uhttp_client_t* client = &((uhttp_client_t*)sv->clients.head)[i];

            if (uhttp_poll(client->sck, &client->events) == 0)
            {
                uhttp_client_event(client);
            }
        }
    }

    return 0;
}


void uhttp_server_close_client(uhttp_client_t* client)
{
    // Find client object in server.
    uhttp_server_t* sv = client->sv;

    size_t i = 0;
    for (i = 0; i < sv->clients.nlen; i++)
    {
        if (((uhttp_client_t*)sv->clients.head)[i].sck == client->sck)
            break;
    }

    // Return if not found.
    if (i >= sv->clients.nlen) return;

    // Close client.
    uhttp_client_destroy(&((uhttp_client_t*)sv->clients.head)[i]);

    // Remove client.
    uhttp_list_remove(&sv->clients, i);
}

UHTTP_EXTERN int uhttp_stop(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Close main socket.
    uhttp_close(sv->sck);

    // Close all clients.
    for (size_t i = 0; i < sv->clients.nlen; i++)
    {
        uhttp_client_t* client = &((uhttp_client_t*)sv->clients.head)[i];
        uhttp_client_destroy(client);
    }
    uhttp_list_clear(&sv->clients);

    return 0;
}
