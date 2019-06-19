#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <limits.h>
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
#define TTL_DEFAULT 20
#define ICMP_MSG_SIZE_DEFAULT 56
#define TIME_INTERVAL_DEFAULT 1
#define SEC_IN_US 1000000
#define SEC_IN_MS 1000
#define MAX_PACKET_SIZE                                                        \
    USHRT_MAX - sizeof(struct icmphdr) - sizeof(struct iphdr)

typedef struct s_pingStat
{
    uint64_t nbrSent;
    uint64_t nbrRecv;
    double rttMin;
    double rttMax;
    double sum;
    struct timeval currSendTs;
    struct timeval currRecvTs;
} t_pingStat;

typedef struct s_option
{
    uint8_t displayUsage;
    int32_t ttl;
    struct timeval timeout;
    uint16_t icmpMsgSize;
    char const *toPing;

} t_option;

typedef struct s_dest
{
    struct addrinfo *addrDest;
    char fqdn[NI_MAXHOST];
    char ip[INET_ADDRSTRLEN];
    int32_t dispFqdn;
} t_dest;

typedef struct s_response
{
    struct sockaddr_in addr;
    struct iovec iovec[1];
    struct msghdr msgHdr;
    uint8_t iovecBuff[USHRT_MAX];
} t_response;

typedef struct s_env
{
    int32_t socket;
    uint32_t packetSize;
    struct addrinfo *resolved;
    t_option opt;
    t_dest dest;
} t_env;

typedef struct s_loopControl
{
    uint8_t wait;
    uint8_t loop;
} t_loopControl;

t_loopControl g_loopControl;

// init_network.c
uint8_t getValidIp(struct addrinfo const *list, struct addrinfo **dest);
struct addrinfo *resolveAddr(char const *addr);
uint8_t getFqdn(char *fqdn, uint64_t fqdnSize, struct addrinfo const *addr);
int32_t initSocket(t_option const *opt);

// loop.c
void stopLoop(int32_t sig);
void stopWait(int32_t sig);
void loop(t_env const *e);

// headers.c
void setHdr(uint8_t *buff,
            t_option const *opt,
            t_dest const *dest,
            uint64_t seq);
void setupMsghdr(t_response *resp);
uint16_t computeChecksum(uint16_t const *ptr, uint16_t packetSize);
uint16_t swap_uint16(uint16_t val);

// opt.c
void parseOptions(t_option *opt, int argc, char const **argv);
void displayUsage();
#endif
