#include "ft_ping.h"

void
dbg_printListAddrInfo(struct addrinfo const *dest)
{
    if (!dest) {
        printf("%s\n", "Nothing to print");
        return;
    }
    printf("%s\n", "Printing Resolved AddrInfo");
    while (dest) {
        struct sockaddr_in *saddr = (struct sockaddr_in *)dest->ai_addr;
        printf("IP = %s | Name = %s\n----------\n",
               inet_ntoa(saddr->sin_addr),
               dest->ai_canonname);
        dest = dest->ai_next;
    }
}

void
dbg_printAddrInfo(struct addrinfo const *dest)
{
    if (!dest) {
        printf("%s\n", "Nothing to print");
        return;
    }
    struct sockaddr_in *saddr = (struct sockaddr_in *)dest->ai_addr;
    printf("IP = %s | Name = %s\n----------\n",
           inet_ntoa(saddr->sin_addr),
           dest->ai_canonname);
}

uint8_t
getValidIp(struct addrinfo const *list, struct addrinfo **dest)
{
    if (!dest || !list) {
        return (1);
    }
    while (list) {
        struct sockaddr_in *saddr = (struct sockaddr_in *)list->ai_addr;
        if (saddr->sin_addr.s_addr) {
            *dest = (struct addrinfo *)list;
            return (0);
        }
        list = list->ai_next;
    }
    return (1);
}

struct addrinfo *
resolveAddr(char const *addr)
{
    struct addrinfo *dest = NULL;
    struct addrinfo hints = { 0 };

    if (!addr) {
        return (NULL);
    }
    hints.ai_flags = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_RAW;
    if (getaddrinfo(addr, NULL, &hints, &dest)) {
        return (NULL);
    }
    return (dest);
}

int32_t
initSocket(t_option const *opt)
{
    int32_t sock = -1;

    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 3) {
        printf("%s\n", "Error initializing socket : maybe you should sudo");
        return (-1);
    }
    // Timeout
    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &opt->timeout,
                   sizeof(struct timeval))) {
        printf("%s\n", "Error setting timeout");
        close(sock);
        return (-1);
    }
    // TTL
    if (setsockopt(sock, SOL_SOCKET, IP_TTL, &opt->ttl, sizeof(int32_t))) {
        printf("%s\n", "Error setting ttl");
        close(sock);
        return (-1);
    }
    return (sock);
}
