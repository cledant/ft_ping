#include "ft_ping.h"
#include <linux/icmp.h>

static inline uint64_t
convertTime(struct timeval const *ts)
{
    return (ts->tv_sec * SEC_IN_US + ts->tv_usec);
}

static void
displayPingStat(t_pingStat const *ps)
{
    // TODO
    printf("\nTO DO STAT\n");
    (void)ps;
}

static inline double
calcAndStatRtt(t_pingStat *ps)
{
    double rtt = (convertTime(&ps->currRecvTs) - convertTime(&ps->currSendTs)) /
                 (double)SEC_IN_MS;
    if (rtt > ps->rttMax) {
        ps->rttMax = rtt;
    } else if (rtt < ps->rttMin) {
        ps->rttMin = rtt;
    }
    ps->sum += rtt;
    return (rtt);
}

static inline void
handleReply(t_response const *resp,
            t_env const *e,
            uint64_t recvBytes,
            t_pingStat *ps)
{
    double rtt = 0.0;

    // TODO Verbose
    ++ps->nbrRecv;
    if (recvBytes != e->packetSize) {
        printf("Error on packet reception from %s\n", e->opt.toPing);
        return;
    }
    rtt = calcAndStatRtt(ps);

    printf("%lu bytes from %s (%s): imcp_seq=%lu ttl=%u time=%.3f ms\n",
           recvBytes - sizeof(struct iphdr),
           "todo",
           e->dest.ip,
           ps->nbrSent,
           ((struct iphdr *)resp->iovecBuff)->ttl,
           rtt);
}

void
stopLoop(int32_t sig)
{
    (void)sig;
    g_loopControl.loop = 0;
    g_loopControl.wait = 0;
}

void
stopWait(int32_t sig)
{
    (void)sig;
    g_loopControl.wait = 0;
    signal(SIGALRM, stopWait);
}

void
loop(t_env const *e)
{
    t_pingStat ps = { 0 };
    uint8_t packet[e->packetSize];
    uint64_t recvBytes = 0;
    t_response resp = { { 0 }, { { 0 } }, { 0 }, { 0 } };

    setupMsghdr(&resp);
    printf("PING %s (%s) %u(%u) bytes of data.\n",
           e->opt.toPing,
           e->dest.ip,
           e->opt.icmpMsgSize,
           e->packetSize);
    while (g_loopControl.loop) {
        uint8_t shouldRecv = 1;

        setHdr(packet, &e->opt, &e->dest, ps.nbrSent);
        gettimeofday(&ps.currSendTs, NULL);
        if (sendto(e->socket,
                   packet,
                   e->packetSize,
                   0,
                   e->dest.addrDest->ai_addr,
                   e->dest.addrDest->ai_addrlen) <= 0) {
            // TODO Verbose
            printf("Error : can't send packet\n");
            shouldRecv = 0;
        } else {
            ++ps.nbrSent;
        }
        if (shouldRecv) {
            if ((recvBytes = recvmsg(e->socket, &resp.msgHdr, 0)) > 0) {
                gettimeofday(&ps.currRecvTs, NULL);
                handleReply(&resp, e, recvBytes, &ps);
            } else {
                // TODO Verbose
                printf("Timeout\n");
            }
        }
        g_loopControl.wait = 1;
        alarm(TIME_INTERVAL_DEFAULT);
        while (g_loopControl.wait)
            ;
    }
    displayPingStat(&ps);
}