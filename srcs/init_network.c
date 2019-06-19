#include "ft_ping.h"

uint8_t
getValidIp(struct addrinfo const *list, struct addrinfo **dest)
{
    if (!dest || !list) {
        return (1);
    }
    while (list) {
        if (((struct sockaddr_in *)list->ai_addr)->sin_addr.s_addr) {
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
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    if (getaddrinfo(addr, NULL, &hints, &dest)) {
        return (NULL);
    }
    return (dest);
}

uint8_t
getFqdn(char *fqdn, uint64_t fqdnSize, struct addrinfo const *addr)
{
    if (!addr || !fqdn) {
        return (1);
    }
    if (getnameinfo(
          addr->ai_addr, addr->ai_addrlen, fqdn, fqdnSize, NULL, 0, 0)) {
        return (1);
    }
    return (0);
}

int32_t
initSocket(t_option const *opt)
{
    int32_t sock = -1;
    uint8_t set = 1;

    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 3) {
        printf("ft_ping : Error initializing socket : maybe you should sudo\n");
        return (-1);
    }
    // Timeout
    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &opt->timeout,
                   sizeof(struct timeval))) {
        printf("ft_ping : Error setting timeout params\n");
        close(sock);
        return (-1);
    }
    // Manual Ip header
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &set, sizeof(uint8_t))) {
        printf("ft_ping : Error setting socket ip header\n");
        close(sock);
        return (-1);
    }
    return (sock);
}
