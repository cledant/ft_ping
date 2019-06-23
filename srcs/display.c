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

    // TODO Check format for float
    if (e->opt.icmpMsgSize >= 16 && e->dest.dispFqdn) {
        printf("%lu bytes from %s (%s): imcp_seq=%lu ttl=%u time=%.3g ms\n",
               recvBytes - sizeof(struct iphdr),
               e->dest.fqdn,
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl,
               rtt);
    } else if (e->opt.icmpMsgSize >= 16) {
        printf("%lu bytes from %s: imcp_seq=%lu ttl=%u time=%.3g ms\n",
               recvBytes - sizeof(struct iphdr),
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl,
               rtt);
    } else if (e->dest.dispFqdn) {
        printf("%lu bytes from %s: imcp_seq=%lu ttl=%u\n",
               recvBytes - sizeof(struct iphdr),
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl);
    } else {
        printf("%lu bytes from %s: imcp_seq=%lu ttl=%u\n",
               recvBytes - sizeof(struct iphdr),
               e->dest.ip,
               ps->nbrSent,
               ((struct iphdr *)resp->iovecBuff)->ttl);
    }
}