#include "ft_ping.h"

void
setHdr(uint8_t *buff, t_option const *opt, uint64_t seq)
{
    struct icmphdr *icmpHdr = (struct icmphdr *)buff;
    uint8_t *msg = (uint8_t *)icmpHdr + sizeof(struct icmphdr);

    if (opt->icmpMsgSize) {
        memset(msg, 42, opt->icmpMsgSize);
    }
    setIcmpHdr(icmpHdr, seq);
    icmpHdr->checksum = computeChecksum(
      (uint16_t *)icmpHdr, opt->icmpMsgSize + sizeof(struct icmphdr));
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

inline uint16_t
swap_uint16(uint16_t val)
{
    return ((val << 8) | (val >> 8));
}