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
displayPingStat(t_pingStat const *ps, char const *addr, uint64_t deadline)
{
    double packetLoss = (1.0 - (ps->nbrRecv / (double)(ps->nbrSent))) * 100.0;
    double avg = ps->sum / (double)ps->nbrRecv;
    double mdev = calcMdev(ps->sum2, ps->sum, ps->nbrRecv);

    //TODO DISPLAY DUPLICATED AND CORRUPTED STAT
    printf("\n--- %s ping statistics ---\n", addr);
    if (!ps->nbrError) {
        printf("%lu packets transmitted, %lu received, %.4g%% packet loss, "
               "time %lums\n",
               ps->nbrSent,
               ps->nbrRecv,
               packetLoss,
               ps->totalTime / SEC_IN_MS);
    } else {
        printf("%lu packets transmitted, %lu received, +%lu errors, %.4g%% "
               "packet loss, time %lums\n",
               ps->nbrSent,
               ps->nbrRecv,
               ps->nbrError,
               packetLoss,
               ps->totalTime / SEC_IN_MS);
    }
    if (ps->nbrRecv) {
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms",
               ps->rttMin,
               avg,
               ps->rttMax,
               mdev);
    }
    if (deadline) {
        struct timeval end;

        gettimeofday(&end, NULL);
        double ewma = ps->totalTime / ((double)ps->nbrSent * SEC_IN_MS);
        double ipg = (convertTime(&end) - ps->startTime) / (double)SEC_IN_US;
        printf(", ipg/ewma %.3f/%.3f ms", ipg, ewma);
    }
    printf("\n");
}

void
displayRtt(t_response const *resp,
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
    printf("%ld bytes from", recvBytes - sizeof(struct iphdr));
    if (e->dest.dispFqdn) {
        printf(" %s (%s): imcp_seq=%lu ttl=%u",
               e->dest.fqdn,
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl);
    } else {
        printf(" %s: imcp_seq=%lu ttl=%u",
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl);
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