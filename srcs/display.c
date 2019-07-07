#include "ft_ping.h"

static inline double
calcMdev(double sum2, double sum, double n)
{
    sum2 = sum2 / (double)n;
    sum = sum / (double)n;
    return (sqrt(sum2 - (sum * sum)));
}

void
printIcmpHdr(struct icmphdr const *icmpHdr)
{
    printf("===ICMP HEADER VALUES===\n\tType: %u\n\tCode: %u\n\tPid: "
           "%u\n\tSequence: %u\n\tChecksum: %u\n----------\n",
           icmpHdr->type,
           icmpHdr->code,
           swapUint16(icmpHdr->un.echo.id),
           swapUint16(icmpHdr->un.echo.sequence),
           icmpHdr->checksum);
}

void
displayPingStat(t_pingStat const *ps, char const *addr)
{
    double packetLoss = (1.0 - (ps->nbrRecv / (double)(ps->nbrSent))) * 100.0;
    double avg = ps->sum / (double)ps->nbrRecv;
    double mdev = calcMdev(ps->sum2, ps->sum, ps->nbrRecv);
    uint64_t diffTime = 0;

    printf("\n--- %s ping statistics ---\n", addr);
    printf("%lu packets transmitted, %lu received, ", ps->nbrSent, ps->nbrRecv);
    if (ps->nbrDuplicated) {
        printf("+%lu duplicates, ", ps->nbrDuplicated);
    }
    if (ps->nbrCorrupted) {
        printf("+%lu corrupted, ", ps->nbrCorrupted);
    }
    if (ps->nbrError) {
        printf("+%lu errors, ", ps->nbrError);
    }
    diffTime = (ps->totalTime > ps->theoricTotalTime)
                 ? ps->totalTime - ps->theoricTotalTime
                 : 0;
    printf(
      "%.4g%% packet loss, time %lums\n", packetLoss, diffTime / SEC_IN_MS);
    if (ps->nbrRecv) {
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms",
               ps->rttMin,
               avg,
               ps->rttMax,
               mdev);
    }
    printf("\n");
}

void
displayRtt(struct iphdr const *ipHdr,
           t_env const *e,
           int64_t recvBytes,
           t_pingStat *ps)
{
    double rtt = calcAndStatRtt(ps);

    if (e->opt.quiet || e->opt.flood) {
        return;
    }
    if (e->opt.printTs) {
        struct timeval ts;
        gettimeofday(&ts, NULL);
        printf("[%lu.%06lu] ", ts.tv_sec, ts.tv_usec);
    }
    printf("%ld bytes from", (int64_t)(recvBytes - sizeof(struct iphdr)));
    if (e->dest.dispFqdn) {
        printf(" %s (%s): imcp_seq=%lu ttl=%u",
               e->dest.fqdn,
               e->dest.ip,
               ps->nbrSent,
               ipHdr->ttl);
    } else {
        printf(" %s: imcp_seq=%lu ttl=%u", e->dest.ip, ps->nbrSent, ipHdr->ttl);
    }
    if (e->opt.icmpMsgSize >= 16) {
        if (rtt > 100) {
            printf(" time=%.0f ms\n", rtt);
        } else if (rtt > 10) {
            uint64_t decimal = (rtt - (uint64_t)rtt) * 10;
            printf(" time=%.0f.%01lu ms\n", rtt, decimal);
        } else if (rtt > 1) {
            uint64_t decimal = (rtt - (uint64_t)rtt) * 100;
            printf(" time=%.0f.%02lu ms\n", rtt, decimal);
        } else {
            uint64_t decimal = (rtt - (uint64_t)rtt) * 1000;
            printf(" time=%.0f.%03lu ms\n", rtt, decimal);
        }
    } else {
        printf("\n");
    }
}