#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIMEOUT_DEFAULT 1
#define TTL_DEFAULT 52
#define ICMP_MSG_SIZE_DEFAULT 56

typedef struct s_pingStat
{
    uint16_t loop;
    uint16_t loopNbr;
} t_pingStat;

typedef struct s_option
{
    int32_t ttl;
    struct timeval timeout;
    uint16_t icmpMsgSize;
} t_option;

typedef struct s_env
{
    int32_t socket;
    struct addrinfo *resolved;
    struct addrinfo *dest;
    t_option opt;
} t_env;

// init.c
void dbg_printListAddrInfo(struct addrinfo const *dest);
void dbg_printAddrInfo(struct addrinfo const *dest);
uint8_t getValidIp(struct addrinfo const *list, struct addrinfo **dest);
struct addrinfo *resolveAddr(char const *addr);
int32_t initSocket(t_option const *opt);

// loop.c
t_pingStat *getPingStat();
void stopLoop(int32_t sig);
void setIcmpHdr(struct icmphdr *hdr, uint16_t seq);
uint16_t computeChecksum(uint16_t const *ptr, uint16_t packetSize);
void loop(t_env const *e);
uint16_t swap_uint16(uint16_t val);

#endif
