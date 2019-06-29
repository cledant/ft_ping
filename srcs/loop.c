#include "ft_ping.h"
#include <linux/icmp.h>

inline double
calcAndStatRtt(t_pingStat *ps)
{
    double rtt = (convertTime(&ps->currRecvTs) - convertTime(&ps->currSendTs)) /
                 (double)SEC_IN_MS;

    if (ps->nbrRecv == 1) {
        ps->rttMin = rtt;
    }
    if (rtt > ps->rttMax) {
        ps->rttMax = rtt;
    } else if (rtt < ps->rttMin) {
        ps->rttMin = rtt;
    }
    ps->sum += rtt;
    ps->sum2 += rtt * rtt;
    return (rtt);
}

inline uint64_t
convertTime(struct timeval const *ts)
{
    return (ts->tv_sec * SEC_IN_US + ts->tv_usec);
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
loop(t_env const *e, uint64_t startTime)
{
    t_pingStat ps = { 0 };
    ps.startTime = startTime;
    uint8_t packet[e->packetSize];
    uint64_t recvBytes = 0;
    t_response resp = { { 0 }, { { 0 } }, { 0 }, { 0 } };

    setupRespBuffer(&resp);
    printf("PING %s (%s) %u(%u) bytes of data.\n",
           e->dest.addrDest->ai_canonname,
           e->dest.ip,
           e->opt.icmpMsgSize,
           e->packetSize);
    if (e->opt.flood && !e->opt.quiet) {
        write(1, &("."), 1);
    }
    while (g_loopControl.loop) {
        uint8_t shouldRecv = 1;
        uint64_t loopTime =
          (convertTime(&ps.currTotalTs) - convertTime(&ps.currSendTs));

        if (e->opt.deadline && (ps.totalTime + loopTime) > e->opt.deadline) {
            displayPingStat(&ps, e->opt.toPing, e->opt.deadline);
            return;
        }
        ps.totalTime += loopTime;
        setHdr(packet, &e->opt, &e->dest, ps.nbrSent);
        gettimeofday(&ps.currSendTs, NULL);
        if (sendto(e->socket,
                   packet,
                   e->packetSize,
                   0,
                   e->dest.addrDest->ai_addr,
                   e->dest.addrDest->ai_addrlen) <= 0) {
            // TODO Verbose + Error checking
            printf("ft_ping: can't send packet\n");
            shouldRecv = 0;
        }
        ++ps.nbrSent;
        if (shouldRecv) {
            if ((recvBytes = recvmsg(e->socket, &resp.msgHdr, 0)) > 0) {
                gettimeofday(&ps.currRecvTs, NULL);
                // TODO Verbose + Error checking
                if (recvBytes != e->packetSize) {
                    printf("ft_ping: Error on packet reception from %s\n",
                           e->opt.toPing);
                } else {
                    ++ps.nbrRecv;
                    displayRtt(&resp, e, recvBytes, &ps);
                }
            } else {
                if (e->opt.verbose) {
                    printf("ft_ping: Packet Timeout\n");
                }
            }
        }
        g_loopControl.wait = 1;
        alarm(TIME_INTERVAL_DEFAULT);
        while (g_loopControl.wait && !e->opt.flood)
            ;
        gettimeofday(&ps.currTotalTs, NULL);
    }
    displayPingStat(&ps, e->opt.toPing, e->opt.deadline);
}