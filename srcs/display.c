#include "ft_ping.h"

static inline double
calcMdev(double sum2, double sum, double n)
{
    sum2 = sum2 / (double)n;
    sum = sum / (double)n;
    return (sqrt(sum2 - (sum * sum)));
}

void
displayPingStat(t_pingStat const *ps, char const *addr)
{
    double packetLoss = (1.0 - (ps->nbrRecv / (double)(ps->nbrSent))) * 100.0;
    double avg = ps->sum / (double)ps->nbrRecv;
    double mdev = calcMdev(ps->sum2, ps->sum, ps->nbrRecv);

    printf("--- %s ping statistics ---\n", addr);
    if (!ps->nbrError) {
        printf("%lu packets transmitted, %lu received, %.4g%% packet loss, "
               "time %lums\n",
               ps->nbrSent,
               ps->nbrRecv,
               packetLoss,
               (uint64_t)ps->totalTime);
    } else {
        printf("%lu packets transmitted, %lu received, +%lu errors, %.4g%% "
               "packet loss, time %lums\n",
               ps->nbrSent,
               ps->nbrRecv,
               ps->nbrError,
               packetLoss,
               (uint64_t)ps->totalTime);
    }
    if (ps->nbrRecv) {
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               ps->rttMin,
               avg,
               ps->rttMax,
               mdev);
    }
}

void
displayRtt(t_response const *resp,
           t_env const *e,
           uint64_t recvBytes,
           t_pingStat *ps)
{
    double rtt = calcAndStatRtt(ps);

    if (e->opt.quiet) {
        return;
    }
    if (e->opt.printTs) {
        struct timeval ts;
        gettimeofday(&ts, NULL);
        printf("[%lu.%06lu] ", ts.tv_sec, ts.tv_usec);
    }
    printf("%lu bytes from", recvBytes - sizeof(struct iphdr));
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