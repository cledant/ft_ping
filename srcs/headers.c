#include "ft_ping.h"

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