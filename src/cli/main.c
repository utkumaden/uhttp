#define _UHTTP_INTERNAL_
#include "uhttp.h"
#include "debug.h"

#include <string.h>
#include <stdio.h>

uhttp_server_t* server;
int spin = 1;

int main(int argc, char** argv)
{
    uhttp_socket_init();

    server = uhttp_create();

    uhttp_option_arg_t arg;
    arg.addr.domain = UHTTP_SOCKET_DOMAIN_INET4;
    arg.addr.port = 8080;
    arg.addr.address[0] = 127;
    arg.addr.address[1] = 0;
    arg.addr.address[2] = 0;
    arg.addr.address[3] = 1;
    uhttp_setoption(server, UHTTP_OPTION_BIND_ADDR, &arg);

    uhttp_start(server);

    while (spin)
    {
        uhttp_pollevents(server);
    }

    uhttp_stop(server);

    uhttp_socket_deinit();
    return 0;
}