#include "ft_ping.h"

void
stopLoop(int32_t sig)
{
    (void)sig;
    g_loopControl.loop = 0;
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
    struct sockaddr from;
    socklen_t fromLen = sizeof(struct sockaddr);

    printf("PING %s (%s) %u(%lu) bytes of data.\n",
           e->opt.toPing,
           e->ip,
           e->opt.icmpMsgSize,
           e->packetSize + sizeof(struct iphdr));
    while (g_loopControl.loop) {
        uint8_t shouldRecv = 1;

        setHdr(packet, &e->opt, ps.nbrSent);
        if (sendto(e->socket,
                   packet,
                   e->packetSize,
                   0,
                   e->dest->ai_addr,
                   e->dest->ai_addrlen) <= 0) {
            printf("Error : can't send packet\n");
            shouldRecv = 0;
        }
        ++ps.nbrSent;
        if (shouldRecv) {
            if (recvfrom(e->socket, packet, e->packetSize, 0, &from, &fromLen) >
                0) {
                printf("Received something from %s\n", e->fqdn);
            } else {
                printf("Timeout\n");
            }
        }
        g_loopControl.wait = 1;
        alarm(TIME_INTERVAL_DEFAULT);
        while (g_loopControl.wait)
            ;
    }
}