#if _WIN32
#define _UHTTP_INTERNAL_
#include "uhttp.h"

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

UHTTP_EXTERN uhttp_sock_t uhttp_socket(uhttp_addr_t* addr)
{
    if (addr == NULL) goto invalid;

    switch (addr->domain)
    {
    case UHTTP_SOCKET_DOMAIN_INET4:
    {
        uhttp_sock_t sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

UHTTP_EXTERN int uhttp_listen(uhttp_sock_t sock, int backlog)
{
    return listen(sock, backlog);
}

UHTTP_EXTERN int uhttp_async(uhttp_sock_t sock, int flag)
{
    u_long xflag = flag;
    return ioctlsocket(sock, FIONREAD, &xflag);
}

UHTTP_EXTERN uhttp_sock_t uhttp_accept(uhttp_sock_t sock, uhttp_addr_t* addr)
{
    struct sockaddr xaddr;
    int len = sizeof(xaddr);

    uhttp_sock_t xsck = accept(sock, &xaddr, &len);

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

UHTTP_EXTERN int uhttp_poll(const uhttp_sock_t* socks, uhttp_event_t* events, size_t count)
{
    if (socks == NULL || events == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    struct pollfd *xpoll = _malloca(count * sizeof(struct pollfd));

    if (xpoll == NULL)
    {
        errno = ENOMEM;
        return -1;
    }

    for (size_t i = 0; i < count; i++)
    {
        xpoll[i].fd = socks[i];
        xpoll[i].events = POLLERR | POLLHUP | POLLIN;
    }

    int error = WSAPoll(xpoll, count, 0);
    if (error)
    {
        errno = EFAULT;
        return -1;
    }

    for (size_t i = 0; i < count; i++)
    {
        events[i] = 0;
        if (xpoll->revents & POLLHUP)  events[i] |= UHTTP_EVENT_HANGUP;
        if (xpoll->revents & POLLERR)  events[i] |= UHTTP_EVENT_ERROR;
        if (xpoll->revents & POLLIN) events[i] |= UHTTP_EVENT_RECEIVE;
    }

    _freea(xpoll);
    return 0;
}

UHTTP_EXTERN ssize_t uhttp_recv(uhttp_sock_t sock, void* buffer, size_t len)
{
    return recv(sock, buffer, len, 0);
}

UHTTP_EXTERN ssize_t uhttp_send(uhttp_sock_t sock, const void* buffer, size_t len)
{
    return send(sock, buffer, len, 0);
}

UHTTP_EXTERN void uhttp_close(uhttp_sock_t sock)
{
    closesocket(sock);
}

#endif
