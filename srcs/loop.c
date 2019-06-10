#include "ft_ping.h"

t_ping_stat *
getPingStat()
{
    static t_ping_stat ps = { 1, 0 };

    return (&ps);
}

void
stopLoop(int signal)
{
    if (signal != SIGINT) {
        return;
    }
    getPingStat()->loop = 0;
}

uint8_t
initIcmpPacket(struct iphdr *packet)
{
    (void)packet;
    return (0);
}

void
loop(const t_env *e)
{
    t_ping_stat *ps = getPingStat();
    (void)e;

    while (ps->loop) {
        sleep(1);
        printf("%s\n", "Loop....");
        ++ps->loopNbr;
    }
    printf("Loop nbrs = %lu\n", ps->loopNbr);
}