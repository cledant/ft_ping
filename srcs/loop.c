#include "ft_ping.h"
#include <linux/icmp.h>

static inline uint8_t
checkIcmpHdrChecksum(t_response *resp, uint8_t verbose, int64_t recvBytes)
{
    struct icmphdr *icmpHdr =
      (struct icmphdr *)(resp->iovecBuff + sizeof(struct iphdr));

    uint16_t recvChecksum = icmpHdr->checksum;
    icmpHdr->checksum = 0;
    icmpHdr->checksum =
      computeChecksum((uint16_t *)icmpHdr, recvBytes - sizeof(struct iphdr));
    if (icmpHdr->checksum == recvChecksum) {
        return (0);
    }
    if (verbose) {
        printf("ft_ping : invalid icmpHdr checksum\n");
        printf("Received icmp checksum: %u | Calculated: %u\n",
               recvChecksum,
               icmpHdr->checksum);
    }
    return (1);
}

static inline uint8_t
checkIpHdrChecksum(t_response *resp, uint8_t verbose, int64_t recvBytes)
{
    struct iphdr *ipHdr = (struct iphdr *)resp->iovecBuff;

    uint16_t recvChecksum = ipHdr->check;
    ipHdr->check = 0;
    ipHdr->check = computeChecksum((uint16_t *)ipHdr, recvBytes);
    if (ipHdr->check == recvChecksum) {
        return (0);
    }
    if (verbose) {
        printf("ft_ping : invalid ipHdr checksum\n");
        printf("Received ip checksum: %u | Calculated: %u\n",
               recvChecksum,
               ipHdr->check);
    }
    return (1);
}

static inline uint8_t
processResponse(t_response *resp,
                t_env const *e,
                int64_t recvBytes,
                t_pingStat *ps)
{
    struct icmphdr *icmpHdr =
      (struct icmphdr *)(resp->iovecBuff + sizeof(struct iphdr));
    if (e->opt.verbose) {
        printIcmpHdr(icmpHdr);
    }
    if (recvBytes < 0 && e->opt.verbose) {
        printf("ft_ping: recvmsg : Network is unreachable\n");
        ++ps->nbrError;
        return (0);
    } else if (recvBytes >= 0 && recvBytes < MIN_PACKET_SIZE &&
               e->opt.verbose) {
        printf("ft_ping: Invalid packet size : %ld reception from %s\n",
               recvBytes,
               e->opt.toPing);
        ++ps->nbrError;
        return (0);
    }
    if (checkIpHdrChecksum(resp, e->opt.verbose, recvBytes)) {
        ++ps->nbrError;
        return (0);
    }
    if (checkIcmpHdrChecksum(resp, e->opt.verbose, recvBytes)) {
        ++ps->nbrError;
        return (0);
    }
    if (icmpHdr->type != ICMP_ECHOREPLY) {
        if (e->opt.verbose) {
            printf("ft_ping : not a echo reply\n");
        }
        ++ps->nbrError;
        return (0);
    }
    if (swapUint16(icmpHdr->un.echo.id) != getpid()) {
        if (e->opt.verbose) {
            printf("ft_ping : invalid pid\n");
        }
        return (1);
    }
    ++ps->nbrRecv;
    displayRtt(resp, e, recvBytes, ps);
    return (0);
}

static inline void
displayWhoIsPing(t_env const *e)
{
    printf("PING %s (%s) %u(%u) bytes of data.\n",
           e->dest.addrDest->ai_canonname,
           e->dest.ip,
           e->opt.icmpMsgSize,
           e->packetSize);
    if (e->opt.flood && !e->opt.quiet) {
        write(1, &("."), 1);
    }
}

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
    t_response resp = { { 0 }, { { 0 } }, { 0 }, { 0 } };

    setupRespBuffer(&resp);
    displayWhoIsPing(e);
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
        int64_t sendBytes = sendto(e->socket,
                                   packet,
                                   e->packetSize,
                                   0,
                                   e->dest.addrDest->ai_addr,
                                   e->dest.addrDest->ai_addrlen);
        if (sendBytes < 0) {
            printf("ft_ping: sendto: Network is unreachable\n");
            shouldRecv = 0;
        }
        if (e->opt.verbose && sendBytes >= 0 && sendBytes != e->packetSize) {
            printf("ft_ping: Invalid size sent. Sent %ld bytes. Should have "
                   "sent %u bytes\n",
                   sendBytes,
                   e->packetSize);
        }
        ++ps.nbrSent;
        if (shouldRecv) {
            while (1) {
                int64_t recvBytes = recvmsg(e->socket, &resp.msgHdr, 0);
                gettimeofday(&ps.currRecvTs, NULL);
                if (!processResponse(&resp, e, recvBytes, &ps))
                    break;
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