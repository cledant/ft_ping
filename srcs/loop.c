#include "ft_ping.h"

t_pingStat *
getPingStat()
{
    static t_pingStat ps = { 0 };

    return (&ps);
}

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

    while (g_loopControl.loop) {
        uint8_t shouldRecv = 1;

        setHdr(packet, &e->opt, ps.nbrSent);
        if (sendto(e->socket,
                   packet,
                   e->packetSize,
                   0,
                   e->dest->ai_addr,
                   e->dest->ai_addrlen) <= 0) {
            printf("%s\n", "Error : can't send packet");
            shouldRecv = 0;
        }
        ++ps.nbrSent;
        if (shouldRecv) {
            if (recvfrom(e->socket, packet, e->packetSize, 0, &from, &fromLen) >
                0) {
                printf("%s\n", "Received something");
            } else {
                printf("%s\n", "Timeout");
            }
        }
        g_loopControl.wait = 1;
        alarm(TIME_INTERVAL_DEFAULT);
        while (g_loopControl.wait)
            ;
    }
}