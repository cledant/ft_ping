#include "ft_ping.h"
#include <linux/icmp.h>

t_pingStat *
getPingStat()
{
    static t_pingStat ps = { 1, 0 };

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

void
setIcmpHdr(struct icmphdr *hdr, uint8_t const *msg, uint16_t seq)
{
    (void)msg;
    hdr->type = ICMP_ECHO;
    hdr->code = 0;
    hdr->un.echo.id = getpid();
    hdr->un.echo.sequence = seq;
    hdr->checksum = checksum();
}

uint16_t
checksum()
{
    return (0);
}

void
loop(t_env const *e)
{
    t_pingStat *ps = getPingStat();
    uint8_t packet[e->opt.icmpMsgSize + sizeof(struct icmphdr)];
    uint8_t *msg = packet + sizeof(struct icmphdr);

    while (ps->loop) {
        if (e->opt.icmpMsgSize) {
            memset(msg, 42, e->opt.icmpMsgSize);
        }
        setIcmpHdr((struct icmphdr *)packet, msg, ps->loopNbr);
        sleep(1);
        printf("%s\n", "Loop....");
        ++ps->loopNbr;
    }
    printf("Loop nbrs = %u\n", ps->loopNbr);
}