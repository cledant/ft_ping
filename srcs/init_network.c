#include "ft_ping.h"

static void
displayAiError(int32_t error)
{
    switch (error) {
        case EAI_AGAIN:
            printf(
              "ft_ping: Name server returned a temporary failure indication\n");
            break;
        case EAI_BADFLAGS:
            printf("ft_ping: Invalid flags\n");
            break;
        case EAI_FAIL:
            printf(
              "ft_ping: Name server returned a permanent failure indication\n");
            break;
        case EAI_FAMILY:
            printf("ft_ping: Unsupported address family\n");
            break;
        case EAI_MEMORY:
            printf("ft_ping: Out of memory\n");
            break;
        case EAI_NONAME:
            printf("ft_ping: Node or service is not known\n");
            break;
        case EAI_SERVICE:
            printf(
              "ft_ping: Service is unavailable for requested socket type.\n");
            break;
        case EAI_OVERFLOW:
            printf("ft_ping: Overflow\n");
            break;
        case EAI_SOCKTYPE:
            printf("ft_ping: Unsupported socket type\n");
            break;
        case EAI_SYSTEM:
            printf("ft_ping: Other system error\n");
            break;
        default:
            printf("ft_ping: Other Ai error\n");
            break;
    }
}

uint8_t
getValidIp(struct addrinfo const *list, struct addrinfo **dest, uint8_t verbose)
{
    if (!dest || !list) {
        return (1);
    }
    uint64_t i = 0;
    while (list) {
        if (((struct sockaddr_in *)list->ai_addr)->sin_addr.s_addr) {
            *dest = (struct addrinfo *)list;
            return (0);
        } else if (verbose) {
            printf("ft_ping: invalid ip for : %s\n", list->ai_canonname);
        }
        list = list->ai_next;
        ++i;
    }
    if (!i && verbose) {
        printf("ft_ping: getaddrinfo returned an empty list\n");
    }
    return (1);
}

struct addrinfo *
resolveAddr(char const *addr, uint8_t verbose)
{
    struct addrinfo *dest = NULL;
    struct addrinfo hints = { 0 };
    int32_t retValue = 0;

    if (!addr) {
        return (NULL);
    }
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    if ((retValue = getaddrinfo(addr, NULL, &hints, &dest))) {
        if (verbose) {
            displayAiError(retValue);
        }
        return (NULL);
    }
    return (dest);
}

uint8_t
reverseResolveDest(char *fqdn,
                   uint64_t size,
                   struct addrinfo const *addr,
                   uint8_t verbose)
{
    int32_t retValue = 0;

    if (!addr || !fqdn) {
        printf("ft_ping: Can't resolve Fqdn\n");
        return (1);
    }
    if ((retValue = getnameinfo(
           addr->ai_addr, addr->ai_addrlen, fqdn, size, NULL, 0, 0))) {
        if (verbose) {
            displayAiError(retValue);
        }
        printf("ft_ping: Can't reverse resolve Fqdn\n");
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
        printf("ft_ping : Error initializing socket\n");
        return (-1);
    }
    // Timeout
    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &opt->timeout,
                   sizeof(struct timeval))) {
        printf("ft_ping: Error setting timeout params\n");
        close(sock);
        return (-1);
    }
    // Manual Ip header
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &set, sizeof(uint8_t))) {
        printf("ft_ping: Error setting socket ip header\n");
        close(sock);
        return (-1);
    }
    return (sock);
}