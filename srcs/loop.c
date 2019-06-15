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
    (void)signal;
    getPingStat()->loop = 0;
}

void
setIcmpHdr(struct icmphdr *hdr, uint16_t seq)
{
    hdr->type = ICMP_ECHO;
    hdr->code = 0;
    hdr->un.echo.id = swap_uint16(getpid());
    hdr->un.echo.sequence = swap_uint16(seq);
    hdr->checksum = 0;
}

uint16_t
computeChecksum(uint16_t const *ptr, uint16_t packetSize)
{
    uint32_t checksum = 0;
    uint64_t size = packetSize;

    while (size > 1) {
        checksum += *ptr;
        size -= sizeof(uint16_t);
        ++ptr;
    }
    if (size == 1) {
        checksum += *(uint8_t *)ptr;
    }
    checksum = (checksum >> 16) + (checksum & 0xFFFF);
    checksum += (checksum >> 16);
    return (~checksum);
}

void
loop(t_env const *e)
{
    t_pingStat *ps = getPingStat();
    uint8_t packet[e->opt.icmpMsgSize + sizeof(struct icmphdr)];
    uint8_t *msg = packet + sizeof(struct icmphdr);
    struct icmphdr *hdr = (struct icmphdr *)packet;
    struct sockaddr from;
    socklen_t fromLen = sizeof(struct sockaddr);

    while (ps->loop) {
        uint8_t shouldRecv = 1;

        if (e->opt.icmpMsgSize) {
            memset(msg, 42, e->opt.icmpMsgSize);
        }
        setIcmpHdr(hdr, ps->loopNbr);
        hdr->checksum = computeChecksum(
          (uint16_t *)packet, e->opt.icmpMsgSize + sizeof(struct icmphdr));
        if (sendto(e->socket,
                   packet,
                   e->opt.icmpMsgSize + sizeof(struct icmphdr),
                   0,
                   e->dest->ai_addr,
                   e->dest->ai_addrlen) <= 0) {
            printf("%s\n", "Error : can't send packet");
            shouldRecv = 0;
        }
        if (shouldRecv) {
            if (recvfrom(e->socket,
                         packet,
                         e->opt.icmpMsgSize + sizeof(struct icmphdr),
                         0,
                         &from,
                         &fromLen) > 0) {
                printf("%s\n", "Received something");
            } else {
                printf("%s\n", "Timeout");
            }
        }
        sleep(1);
        ++ps->loopNbr;
    }
    printf("Loop nbrs = %u\n", ps->loopNbr);
}

inline uint16_t
swap_uint16(uint16_t val)
{
    return ((val << 8) | (val >> 8));
}