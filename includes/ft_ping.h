#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct env
{
    int32_t socket;
    struct addrinfo *resolved;
    struct addrinfo *dest;
} t_env;

struct addrinfo *resolveAddr(const char *addr);
uint8_t getValidIp(const struct addrinfo *list, struct addrinfo **dest);
int32_t initSocket();

// Debug print
void dbg_printListAddrInfo(const struct addrinfo *dest);
void dbg_printAddrInfo(const struct addrinfo *dest);

#endif
