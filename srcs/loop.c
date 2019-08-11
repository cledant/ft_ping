#include "ft_ping.h"

static inline uint8_t
validateIcmpPacket(t_env const *e,
                   t_pingStat *ps,
                   int64_t recvBytes,
                   uint8_t *ipPacketBuff)
{
    struct icmphdr *icmpHdr =
      (struct icmphdr *)(ipPacketBuff + sizeof(struct iphdr));

    if (checkIpHdrChecksum((struct iphdr *)ipPacketBuff,
                           e->opt.verbose,
                           e->opt.quiet,
                           recvBytes)) {
        ++ps->nbrCorrupted;
        return (0);
    }
    if (checkIcmpHdrChecksum(
          icmpHdr, e->opt.verbose, e->opt.quiet, recvBytes)) {
        ++ps->nbrCorrupted;
        return (0);
    }
    if (icmpHdr->type != ECHOREPLY) {
        if (e->opt.verbose && !e->opt.quiet) {
            printIcmpHdr(
              (struct icmphdr *)(ipPacketBuff + sizeof(struct iphdr)));
            printf("ft_ping : not a echo reply\n");
        }
        if (icmpHdr->type == TTL_ERROR) {
            struct iphdr const *err =
              (struct iphdr *)(ipPacketBuff + MIN_PACKET_SIZE);

            if (swapUint16(err->id) != getpid()) {
                return (1);
            }
            ps->ttlError = 1;
            ++ps->nbrError;
            displayRtt((struct iphdr *)ipPacketBuff, e, recvBytes, ps);
            return (0);
        }
        return (1);
    }
    if (swapUint16(icmpHdr->un.echo.id) != getpid()) {
        if (e->opt.verbose && !e->opt.quiet) {
            printIcmpHdr(
              (struct icmphdr *)(ipPacketBuff + sizeof(struct iphdr)));
            printf("ft_ping : invalid pid\n");
        }
        return (1);
    }
    if (swapUint16(icmpHdr->un.echo.sequence) < (ps->nbrSent - 1)) {
        if (e->opt.verbose && !e->opt.quiet) {
            printIcmpHdr(
              (struct icmphdr *)(ipPacketBuff + sizeof(struct iphdr)));
            printf("ft_ping : duplicated packet\n");
        }
        ++ps->nbrDuplicated;
        return (1);
    }
    ++ps->nbrRecv;
    displayRtt((struct iphdr *)ipPacketBuff, e, recvBytes, ps);
    return (0);
}

static inline uint8_t
processResponse(t_response *resp,
                t_env const *e,
                int64_t recvBytes,
                t_pingStat *ps)
{
    if (recvBytes < 0) {
        if (e->opt.verbose && !e->opt.quiet) {
            printf("ft_ping: recvmsg : Network is unreachable\n");
        }
        return (0);
    } else if (recvBytes >= 0 && recvBytes < MIN_PACKET_SIZE) {
        if (e->opt.verbose && !e->opt.quiet) {
            printf("ft_ping: Invalid packet size : %ld reception from %s\n",
                   recvBytes,
                   e->opt.toPing);
        }
        ++ps->nbrError;
        return (0);
    }
    return (validateIcmpPacket(e, ps, recvBytes, (uint8_t *)resp->iovecBuff));
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

static inline void
checkSendError(t_env const *e,
               int64_t sendBytes,
               uint8_t *shouldRecv,
               t_pingStat *ps)
{
    if (sendBytes < 0) {
        if (!e->opt.quiet) {
            printf("ft_ping: sendto: Network is unreachable\n");
        }
        *shouldRecv = 0;
    }
    if (sendBytes >= 0 && (sendBytes != e->packetSize)) {
        if (e->opt.verbose && !e->opt.quiet) {
            printf("ft_ping: Invalid size sent. Sent %ld bytes. Should have "
                   "sent %u bytes\n",
                   sendBytes,
                   e->packetSize);
        }
        ++ps->nbrError;
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
        int64_t recvBytes = 0;
        uint8_t shouldRecv = 1;
        uint64_t loopTime =
          (convertTime(&ps.currTotalTs) - convertTime(&ps.currSendTs));

        if (e->opt.deadline && (ps.totalTime + loopTime) > e->opt.deadline) {
            displayPingStat(&ps, e->opt.toPing, e->opt.flood, e->packetSize);
            return;
        }
        ps.ewma = (ps.nbrSent == 1) ? loopTime * 8 : loopTime - (ps.ewma / 8);
        ps.totalTime += loopTime;
        if (ps.nbrSent) {
            ps.theoricTotalTime += SEC_IN_US;
        }
        setHdr(packet, &e->opt, &e->dest, ps.nbrSent);
        gettimeofday(&ps.currSendTs, NULL);
        int64_t sendBytes = sendto(e->socket,
                                   packet,
                                   e->packetSize,
                                   0,
                                   e->dest.addrDest->ai_addr,
                                   e->dest.addrDest->ai_addrlen);
        checkSendError(e, sendBytes, &shouldRecv, &ps);
        ++ps.nbrSent;
        if (shouldRecv) {
            while (1) {
                recvBytes = recvmsg(e->socket, &resp.msgHdr, 0);
                gettimeofday(&ps.currRecvTs, NULL);
                if (!processResponse(&resp, e, recvBytes, &ps))
                    break;
            }
        }
        if (!e->opt.flood && recvBytes >= 0) {
            g_loopControl.wait = 1;
            alarm(TIME_INTERVAL_DEFAULT);
            while (g_loopControl.wait) {
            }
        }
        gettimeofday(&ps.currTotalTs, NULL);
    }
    displayPingStat(&ps, e->opt.toPing, e->opt.flood, e->packetSize);
}