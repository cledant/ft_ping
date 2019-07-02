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