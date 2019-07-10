#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <limits.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <math.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIMEOUT_DEFAULT 1
#define TTL_DEFAULT 128
#define ECHOREPLY 0
#define TTL_ERROR 11
#define ICMP_MSG_SIZE_DEFAULT 56
#define TIME_INTERVAL_DEFAULT 1
#define SEC_IN_US 1000000
#define SEC_IN_MS 1000
#define NBR_OPTION 9
#define MAX_TTL USHRT_MAX
#define MIN_PACKET_SIZE (int64_t)(sizeof(struct icmphdr) + sizeof(struct iphdr))
#define MAX_PACKET_SIZE USHRT_MAX - MIN_PACKET_SIZE

typedef struct s_pingStat
{
    uint64_t nbrSent;
    uint64_t nbrRecv;
    uint64_t nbrError;
    uint64_t nbrDuplicated;
    uint64_t nbrCorrupted;
    uint8_t ttlError;
    double rttMin;
    double rttMax;
    double sum;
    double sum2;
    double ewma;
    uint64_t totalTime;
    uint64_t theoricTotalTime;
    uint64_t startTime;
    struct timeval currSendTs;
    struct timeval currRecvTs;
    struct timeval currTotalTs;
} t_pingStat;

typedef struct s_option
{
    uint8_t displayUsage;
    uint8_t verbose;
    uint8_t quiet;
    uint8_t printTs;
    uint8_t noLookup;
    uint8_t flood;
    uint64_t deadline;
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
    volatile uint8_t wait;
    volatile uint8_t loop;
} t_loopControl;

t_loopControl g_loopControl;

// init_network.c
uint8_t getValidIp(struct addrinfo const *list,
                   struct addrinfo **dest,
                   uint8_t verbose);
struct addrinfo *resolveAddr(char const *addr, uint8_t verbose);
uint8_t reverseResolveDest(char *fqdn,
                           uint64_t size,
                           struct addrinfo const *addr,
                           uint8_t verbose);
int32_t initSocket(t_option const *opt);

// loop.c
double calcAndStatRtt(t_pingStat *ps);
void loop(t_env const *e, uint64_t startTime);

// signal.c
void stopLoop(int32_t sig);
void stopWait(int32_t sig);

// utility.c
uint64_t convertTime(struct timeval const *ts);
uint16_t swapUint16(uint16_t val);
uint16_t computeChecksum(uint16_t const *ptr, uint16_t packetSize);

// headers.c
void setHdr(uint8_t *buff,
            t_option const *opt,
            t_dest const *dest,
            uint64_t seq);
void setupRespBuffer(t_response *resp);

// opt.c
void parseOptions(t_option *opt, int32_t argc, char const **argv);
void displayUsage();

// display.c
void printIcmpHdr(struct icmphdr const *icmpHdr);
void displayPingStat(t_pingStat const *ps,
                     char const *addr,
                     uint8_t flood,
                     uint32_t packetSize);
void displayRtt(struct iphdr const *ipHdr,
                t_env const *e,
                int64_t recvBytes,
                t_pingStat *ps);
#endif
