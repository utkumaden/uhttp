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
#include "client.h"

int uhttp_client_create(uhttp_client_t* client)
{
    return 0;
}

void uhttp_client_destroy(uhttp_client_t* client)
{
#if _WIN32
    closesocket(client->sck);
#else
    close(client->sck);
#endif
}

int uhttp_client_event(uhttp_client_t* client)
{
    int revents = client->pollfd->revents;

    if (revents & POLLHUP)
    {
        uhttp_server_close_client(client);
        return 0;
    }
    
    if (revents & POLLERR)
    {
        // TODO:
    }
    
    if (revents & POLLNVAL)
    {
        // TODO:
    }

    return 0;
}
