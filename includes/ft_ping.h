#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <linux/ip.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIMEOUT_DEFAULT 1
#define TTL_DEFAULT 32
#define ICMP_PACKET_SIZE_DEFAULT 64

typedef struct s_ping_stat
{
    uint8_t loop;
    uint64_t loopNbr;
} t_ping_stat;

typedef struct s_option
{
    int32_t ttl;
    struct timeval timeout;
    uint32_t imcp;
} t_option;

typedef struct s_env
{
    int32_t socket;
    struct addrinfo *resolved;
    struct addrinfo *dest;
    t_option opt;
} t_env;

// init.c
void dbg_printListAddrInfo(const struct addrinfo *dest);
void dbg_printAddrInfo(const struct addrinfo *dest);
struct addrinfo *resolveAddr(const char *addr);
uint8_t getValidIp(const struct addrinfo *list, struct addrinfo **dest);
int32_t initSocket(const t_option *opt);

// loop.c
t_ping_stat *getPingStat();
void stopLoop(int32_t sig);
uint8_t initIcmpPacket(struct iphdr *packet);
void loop(const t_env *e);

#endif
