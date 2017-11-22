#define TRACE_MODULE _tcp

#include "core_debug.h"
#include "core_arch_network.h"

status_t tcp_server(sock_id *new,
        int family, const char *hostname, c_uint16_t port)
{
    status_t rv;
    c_sockaddr_t *sa;
    sock_t *sock = NULL;
    char buf[CORE_ADDRSTRLEN];

    rv = core_getaddrinfo(&sa, family, hostname, port, AI_PASSIVE);
    d_assert(rv == CORE_OK && sa, return CORE_ERROR,);

    while(sa)
    {
        rv = sock_create(new, sa->sa.sa_family, SOCK_STREAM, IPPROTO_TCP);
        if (rv != CORE_OK) continue;
        
        sock = (sock_t *)*new;

        d_assert(sock_setsockopt(*new, SOCK_O_REUSEADDR, 1) == CORE_OK,
                return CORE_ERROR,
                "setsockopt(%s:%d) failed(%d:%s)",
                CORE_NTOP(sa, buf), port, errno, strerror(errno));

        if (sock_bind(*new, sa) == CORE_OK)
        {
            d_trace(1, "tcp bind %s:%d\n", CORE_NTOP(sa, buf), port);
            break;
        }

        rv = sock_delete(*new);
        d_assert(rv == CORE_OK, return CORE_ERROR,);

        sa = sa->next;
    }

    if (sa == NULL)
    {
        d_error("tcp bind(%d:%s:%d) failed(%d:%s)",
                sock->family, hostname, port, errno, strerror(errno));
        return CORE_ERROR;
    }

    rv = core_freeaddrinfo(sa);
    d_assert(rv == CORE_OK, return CORE_ERROR,);

    rv = sock_listen(*new);
    d_assert(rv == CORE_OK, return CORE_ERROR,);

    return CORE_OK;
}

status_t tcp_client(sock_id *new,
        int family, const char *hostname, c_uint16_t port)
{
    status_t rv;
    c_sockaddr_t *sa;
    sock_t *sock = NULL;
    char buf[CORE_ADDRSTRLEN];

    rv = core_getaddrinfo(&sa, family, hostname, port, 0);
    d_assert(rv == CORE_OK && sa, return CORE_ERROR,);

    while(sa)
    {
        rv = sock_create(new, sa->sa.sa_family, SOCK_STREAM, IPPROTO_TCP);
        if (rv != CORE_OK) continue;
        
        sock = (sock_t *)*new;

        if (sock_connect(*new, sa) == CORE_OK)
        {
            d_trace(1, "tcp connect %s:%d\n", CORE_NTOP(sa, buf), port);
            break;
        }

        rv = sock_delete(*new);
        d_assert(rv == CORE_OK, return CORE_ERROR,);

        sa = sa->next;
    }

    if (sa == NULL)
    {
        d_error("tcp connect(%d:%s:%d) failed(%d:%s)",
                sock->family, hostname, port, errno, strerror(errno));
        return CORE_ERROR;
    }

    rv = core_freeaddrinfo(sa);
    d_assert(rv == CORE_OK, return CORE_ERROR,);

    return CORE_OK;
}
