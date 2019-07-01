#include "ft_ping.h"
#include <linux/icmp.h>

static inline void
setImcpHeader(struct icmphdr *icmpHdr, uint64_t seq, uint16_t icmpMsgSize)
{
    icmpHdr->type = ICMP_ECHO;
    icmpHdr->code = 0;
    icmpHdr->un.echo.id = swapUint16(getpid());
    icmpHdr->un.echo.sequence = swapUint16(seq);
    icmpHdr->checksum = 0;
    icmpHdr->checksum = computeChecksum((uint16_t *)icmpHdr,
                                        icmpMsgSize + sizeof(struct icmphdr));
}

static inline void
setIpHdr(struct iphdr *ipHdr, t_option const *opt, t_dest const *dest)
{
    ipHdr->version = 4;
    ipHdr->tos = 0;
    ipHdr->ihl = 5;
    ipHdr->tot_len =
      opt->icmpMsgSize + sizeof(struct icmphdr) + sizeof(struct iphdr);
    ipHdr->id = swapUint16(getpid());
    ipHdr->frag_off = 0;
    ipHdr->ttl = opt->ttl;
    ipHdr->protocol = IPPROTO_ICMP;
    ipHdr->check = 0;
    ipHdr->saddr = 0;
    ipHdr->daddr =
      ((struct sockaddr_in *)dest->addrDest->ai_addr)->sin_addr.s_addr;
}

void
setHdr(uint8_t *buff, t_option const *opt, t_dest const *dest, uint64_t seq)
{
    struct iphdr *ipHdr = (struct iphdr *)buff;
    struct icmphdr *icmpHdr = (struct icmphdr *)(buff + sizeof(struct iphdr));
    uint8_t *msg = (uint8_t *)icmpHdr + sizeof(struct icmphdr);

    if (opt->icmpMsgSize) {
        memset(msg, 42, opt->icmpMsgSize);
    }
    setImcpHeader(icmpHdr, seq, opt->icmpMsgSize);
    setIpHdr(ipHdr, opt, dest);
}



void
setupRespBuffer(t_response *resp)
{
    resp->iovec[0].iov_base = resp->iovecBuff;
    resp->iovec[0].iov_len = USHRT_MAX;
    resp->msgHdr.msg_iov = resp->iovec;
    resp->msgHdr.msg_iovlen = 1;
    resp->msgHdr.msg_name = &resp->addr;
    resp->msgHdr.msg_namelen = sizeof(struct sockaddr_in);
    resp->msgHdr.msg_control = NULL;
    resp->msgHdr.msg_controllen = 0;
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
swapUint16(uint16_t val)
{
    return ((val << 8) | (val >> 8));
}