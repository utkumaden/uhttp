#define _UHHTP_INTERNAL_
#include "uhttp.h"

#include <string.h>
#include <stdio.h>

uhttp_server_t* server;
int spin = 1;

int main(int argc, char** argv)
{
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2,0), &data);
#endif

    server = uhttp_create();

    uhttp_option_arg_t arg;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = 8080;
    addr.sin_addr.S_un.S_un_b.s_b1 = 127;
    addr.sin_addr.S_un.S_un_b.s_b2 = 0;
    addr.sin_addr.S_un.S_un_b.s_b3 = 0;
    addr.sin_addr.S_un.S_un_b.s_b4 = 1;

    memcpy(&arg.bind_addr, &addr, sizeof(addr));
    arg.bind_addr.len = sizeof(addr);

    uhttp_setoption(server, UHTTP_OPTION_BIND_ADDR, &arg);

    uhttp_start(server);

    while (spin)
    {
        uhttp_poll(server);
    }

    uhttp_stop(server);

#if _WIN32
    WSACleanup();
#endif
    return 0;
}