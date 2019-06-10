#include "ft_ping.h"

void
dbg_printListAddrInfo(const struct addrinfo *dest)
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
};

void
dbg_printAddrInfo(const struct addrinfo *dest)
{
    if (!dest) {
        printf("%s\n", "Nothing to print");
        return;
    }
    struct sockaddr_in *saddr = (struct sockaddr_in *)dest->ai_addr;
    printf("IP = %s | Name = %s\n----------\n",
           inet_ntoa(saddr->sin_addr),
           dest->ai_canonname);
};

uint8_t
getValidIp(const struct addrinfo *list, struct addrinfo **dest)
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
resolveAddr(const char *addr)
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
initSocket()
{
    int32_t sock = -1;

    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 3) {
        return (-1);
    }
    return (sock);
}