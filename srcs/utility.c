#include "ft_ping.h"

inline uint64_t
convertTime(struct timeval const *ts)
{
    return (ts->tv_sec * SEC_IN_US + ts->tv_usec);
}

inline uint16_t
swapUint16(uint16_t val)
{
    return ((val << 8) | (val >> 8));
}

inline uint16_t
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

inline uint8_t
checkIcmpHdrChecksum(struct icmphdr *icmpHdr,
                     uint8_t verbose,
                     uint8_t quiet,
                     int64_t recvBytes)
{
    uint16_t recvChecksum = icmpHdr->checksum;
    icmpHdr->checksum = 0;
    icmpHdr->checksum =
      computeChecksum((uint16_t *)icmpHdr, recvBytes - sizeof(struct iphdr));
    if (icmpHdr->checksum == recvChecksum) {
        return (0);
    }
    if (verbose && !quiet) {
        printIcmpHdr(icmpHdr);
        printf("ft_ping : invalid icmpHdr checksum\n");
        printf("Received icmp checksum: %u | Calculated: %u\n",
               recvChecksum,
               icmpHdr->checksum);
    }
    return (1);
}

inline uint8_t
checkIpHdrChecksum(struct iphdr *ipHdr,
                   uint8_t verbose,
                   uint8_t quiet,
                   int64_t recvBytes)
{
    uint16_t recvChecksum = ipHdr->check;
    ipHdr->check = 0;
    ipHdr->check = computeChecksum((uint16_t *)ipHdr, recvBytes);
    if (ipHdr->check == recvChecksum) {
        return (0);
    }
    if (verbose && !quiet) {
        printIcmpHdr((struct icmphdr *)(ipHdr + sizeof(struct iphdr)));
        printf("ft_ping : invalid ipHdr checksum\n");
        printf("Received ip checksum: %u | Calculated: %u\n",
               recvChecksum,
               ipHdr->check);
    }
    return (1);
}