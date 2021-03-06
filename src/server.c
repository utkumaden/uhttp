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

#include "client.h"

#include "stdlib.h"
#include "errno.h"
#include "stdio.h"

#if !_WIN32
typedef int SOCKET;
#endif

#define UHTTP_BACKLOG_DEFAULT 16

struct uhttp_server_t
{
    /* Listen socket */
    SOCKET sck;

    /* Length of socket backlog. */
    int backlog;

    /* Bound socket address. */
    struct sockaddr bind_addr;
    unsigned int bind_addr_len;

    /* Client List */
    uhttp_client_t* clients;
    unsigned int clients_len;

    /* Poll List */
    struct pollfd* pollfds;
    unsigned int pollfds_len;

    /* Error function. */
    uhttp_error_func_t on_error;

};

void uhttp_error_default(int number, const char* description)
{
#ifdef _DEBUG
    fprintf(stderr, "uhttp: (errno=%d) %s\n", number, description);
#endif
}

UHTTPAPI uhttp_server_t* uhttp_create()
{
    uhttp_server_t* sv = malloc(sizeof(uhttp_server_t));
    
    if (sv)
    {
        // Initialize sockets list.
        sv->sck = -1;

        // Set backlog to default value.
        sv->backlog = UHTTP_BACKLOG_DEFAULT;

        // Clear bind address.
        memset(&sv->bind_addr, 0, sizeof(struct sockaddr));
        sv->bind_addr_len = 0;

        // Clear client list.
        sv->clients = NULL;
        sv->clients_len = 0;

        // Clear pollfds.
        sv->pollfds = NULL;
        sv->pollfds_len = 0;

        // Set error callback.
        sv->on_error = uhttp_error_default;
    }

    return sv;
}

UHTTPAPI void uhttp_destroy(uhttp_server_t* sv)
{
    if (sv)
    {
        uhttp_stop(sv);
    }

    free(sv);
}

UHTTPAPI int uhttp_setoption(uhttp_server_t* sv, uhttp_option_name_t name, const uhttp_option_arg_t* value)
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
        sv->bind_addr_len = value->bind_addr.len;
        memcpy(&sv->bind_addr, &value->bind_addr.addr, value->bind_addr.len);
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

UHTTPAPI int uhttp_getoption(uhttp_server_t* sv, uhttp_option_name_t name, uhttp_option_arg_t* value)
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
        value->bind_addr.len = sv->bind_addr_len;
        memcpy(&value->bind_addr.addr, &sv->bind_addr, sv->bind_addr_len);
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

UHTTPAPI int uhttp_start(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Allocate listen socket.
    SOCKET sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sck < 0)
    {
        errno = ECANCELED;
        return -1;
    }

    sv->sck = sck;

    // Bind socket.
    if (bind(sck, &sv->bind_addr, sv->bind_addr_len))
    {
        return -1;
    }

    // Listen on socket.
    if (listen(sck, sv->backlog))
    {
        return -1;
    }

#if _WIN32
    u_long opt = 1;
    ioctlsocket(sck, FIONBIO, &opt);
#endif

    return 0;
}

UHTTPAPI int uhttp_poll(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Try accepting new sockets.
    {
        struct sockaddr addr;
        unsigned int len;
        SOCKET nclient = accept(sv->sck, &addr, &len);

        if (nclient >= 0)
        {
            uhttp_client_t* clients = realloc(sv->clients, sizeof(uhttp_client_t) * (sv->clients_len + 1));
            if (clients)
            {
                sv->clients = clients;
                uhttp_client_t* newclient = &sv->clients[sv->clients_len++];

                if (uhttp_client_create(newclient))
                {
                    // Close connection due to error.
                    sv->on_error(errno = ENOMEM, "Could not create client. (uhttp_poll)");
#ifdef _WIN32
                    closesocket(nclient);
#else
                    close(nclient);
#endif
                    newclient = realloc(sv->clients, sizeof(uhttp_client_t) *(sv->clients_len - 1));
                    if (newclient)
                    {
                        sv->clients = newclient;
                    }
                    else
                    {
                        // Memory issue, leak last slot.
                        sv->on_error(errno = ENOMEM, "Downsize clients list failed (uhttp_poll)");
                    }
                    --sv->clients_len;
                }
            }
            else
            {
                // Ram overflow.
#ifdef _WIN32
                closesocket(nclient);
#else
                close(nclient);
#endif

                sv->on_error(errno = ENOMEM, "Could not allocate new clients list (uhttp_poll).");
                return -1;
            }

            struct pollfd* newpollfds = realloc(sv->pollfds, sizeof(struct pollfd) * (sv->pollfds_len + 1));
            if (newpollfds)
            {
                sv->pollfds = newpollfds;
                struct pollfd * pollfd = &newpollfds[sv->pollfds_len++];

                pollfd->fd = nclient;
                pollfd->events = 0;
            }
            else
            {
                sv->on_error((errno = ENOMEM), "Could not expand pollfds list.");
                uhttp_client_destroy(&sv->clients[sv->clients_len - 1]);

                clients = realloc(sv->clients, sizeof(uhttp_client_t) * (sv->clients_len - 1));
                if (clients)
                {
                    sv->clients = clients;
                }
                else
                {
                    sv->on_error((errno = ENOMEM), "Could not shrink clients list");
                }
                --sv->clients_len;
            }
        }
    }

    // Poll all open sockets.
    {
#if _WIN32
        WSAPoll(sv->pollfds, sv->pollfds_len, 0);
#endif

        int error;
        // Call events on all client objects.
        for (int i = 0; i < sv->clients_len; i++)
        {
            error = uhttp_client_event(&sv->clients[i]);
        }
    }

    return 0;
}


void uhttp_server_close_client(uhttp_client_t* client)
{
    // Find client object in server.
    uhttp_server_t* sv = client->server;


    int i = 0;
    for (int i = 0; i < sv->clients_len; i++)
    {
        if (&sv->clients[i] == client) break;
    }

    if (i >= sv->clients_len) return;

    uhttp_client_destroy(client);

    // Remove from clients list.
    memmove(&sv->clients[i], &sv->clients[i + 1], sizeof(uhttp_client_t) * (sv->clients_len - i - 1));
    uhttp_client_t* newclients = realloc(sv->clients, sizeof(uhttp_client_t) * (sv->clients_len - 1));
    if (newclients == NULL)
    {
        // Leak last pointer.
        sv->on_error((errno = ENOMEM), "Realloc failed to downsize clients list, (uhttp_server_close_client)");
    }
    else
    {
        sv->clients = newclients;
    }
    --sv->clients_len;

    // Remove from pollfds.
    memmove(&sv->pollfds[i], &sv->pollfds[i + 1], sizeof(uhttp_client_t) * (sv->clients_len - i - 1));
    struct pollfd* newpollfds = realloc(sv->clients, sizeof(uhttp_client_t) * (sv->clients_len - 1));
    if (newpollfds == NULL)
    {
        // Leak last pointer, again.
        sv->on_error((errno = ENOMEM), "Realloc failed to downsize pollfds list, (uhttp_server_close_client)");
    }
    else
    {
        sv->pollfds = newpollfds;
    }
    --sv->pollfds_len;

    return;
}

UHTTPAPI int uhttp_stop(uhttp_server_t* sv)
{
    if (sv == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Close main socket.
#if _WIN32
    closesocket(sv->sck);
#else
    close(sv->sockets[i]);
#endif

    // Close all clients.
    for (int i = 0; i < sv->clients_len; i++)
    {
        uhttp_client_destroy(&sv->clients[i]);
    }
    free(sv->clients);
    sv->clients = NULL;
    sv->clients_len = 0;
    
    // Close list of open pollfds.
    free(sv->pollfds);
    sv->pollfds_len = 0;

    return 0;
}
