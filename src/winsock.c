#if _WIN32
#define _UHTTP_INTERNAL_
#include "uhttp.h"

#include "debug.h"

#include <errno.h>
#include <malloc.h>

UHTTP_EXTERN int uhttp_socket_init()
{
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 0), &data))
    {
        errno = EFAULT;
        return -1;
    }
    else
    {
        return 0;
    }
}

UHTTP_EXTERN void uhttp_socket_deinit()
{
    WSACleanup();
}

UHTTP_EXTERN uhttp_socket_t uhttp_socket(uhttp_addr_t* addr)
{
    if (addr == NULL) goto invalid;

    switch (addr->domain)
    {
    case UHTTP_SOCKET_DOMAIN_INET4:
    {
        uhttp_socket_t sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sck == INVALID_SOCKET)
        {
            errno = EAGAIN;
            return UHTTP_INVALID_SOCKET;
        }

        struct sockaddr_in sckaddr;
        sckaddr.sin_family = AF_INET;
        sckaddr.sin_port = htons(addr->port);
        sckaddr.sin_addr.S_un.S_un_b.s_b1 = addr->address[0];
        sckaddr.sin_addr.S_un.S_un_b.s_b2 = addr->address[1];
        sckaddr.sin_addr.S_un.S_un_b.s_b3 = addr->address[2];
        sckaddr.sin_addr.S_un.S_un_b.s_b4 = addr->address[3];
        
        if (bind(sck, &sckaddr, sizeof(struct sockaddr_in)))
        {
            errno = EADDRINUSE;
            return UHTTP_INVALID_SOCKET;
        }

        return sck;
    }
    case UHTTP_SOCKET_DOMAIN_INET6:
    {
        errno = EAFNOSUPPORT;
        return UHTTP_INVALID_SOCKET;
    }
    default:
        goto invalid;
    }

invalid:
    errno = EINVAL;
    return UHTTP_INVALID_SOCKET;
}

UHTTP_EXTERN int uhttp_listen(uhttp_socket_t sock, int backlog)
{
    return listen(sock, backlog);
}

UHTTP_EXTERN int uhttp_async(uhttp_socket_t sock, int flag)
{
    u_long xflag = flag;
    return ioctlsocket(sock, FIONBIO, &xflag);
}

UHTTP_EXTERN uhttp_socket_t uhttp_accept(uhttp_socket_t sock, uhttp_addr_t* addr)
{
    struct sockaddr xaddr;
    int len = sizeof(xaddr);

    uhttp_socket_t xsck = accept(sock, &xaddr, &len);

    if (xsck == INVALID_SOCKET)
    {
        return UHTTP_INVALID_SOCKET;
    }

    if (addr)
    {
        if (xaddr.sa_family == AF_INET)
        {
            addr->domain = UHTTP_SOCKET_DOMAIN_INET4;
            addr->port = ntohs(((struct sockaddr_in*)&xaddr)->sin_port);
            addr->address[0] = ((struct sockaddr_in*)&xaddr)->sin_addr.S_un.S_un_b.s_b1;
            addr->address[1] = ((struct sockaddr_in*)&xaddr)->sin_addr.S_un.S_un_b.s_b2;
            addr->address[2] = ((struct sockaddr_in*)&xaddr)->sin_addr.S_un.S_un_b.s_b3;
            addr->address[3] = ((struct sockaddr_in*)&xaddr)->sin_addr.S_un.S_un_b.s_b4;
        }
        else
        {
            memset(addr, 0, sizeof(*addr));
        }
    }

    return xsck;
}

UHTTP_EXTERN int uhttp_poll(uhttp_socket_t sock, uhttp_event_t* events)
{
    struct pollfd pollfd;
    int error;

    pollfd.fd = sock;
    pollfd.events = POLLPRI | POLLRDBAND | POLLRDNORM | POLLWRNORM;

    error = WSAPoll(&pollfd, 1, 0);

    uhttp_log("%d", WSAGetLastError());

    if (error)
    {
        return -1;
    }

    *events = 0;
    if (pollfd.revents & POLLHUP) *events |= UHTTP_EVENT_HANGUP;
    if (pollfd.revents & POLLERR) *events |= UHTTP_EVENT_ERROR;
    if (pollfd.revents & POLLIN) *events |= UHTTP_EVENT_RECEIVE;

    return 0;
}

UHTTP_EXTERN ssize_t uhttp_recv(uhttp_socket_t sock, void* buffer, size_t len)
{
    return recv(sock, buffer, len, 0);
}

UHTTP_EXTERN ssize_t uhttp_send(uhttp_socket_t sock, const void* buffer, size_t len)
{
    return send(sock, buffer, len, 0);
}

UHTTP_EXTERN void uhttp_close(uhttp_socket_t sock)
{
    closesocket(sock);
}

#endif
