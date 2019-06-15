#include "ft_ping.h"

void
parseOptions(t_option *opt, int argc, char const **argv)
{
    // TODO actual option parsing
    *opt = (t_option){
        0, TTL_DEFAULT, { TIMEOUT_DEFAULT, 0 }, ICMP_MSG_SIZE_DEFAULT, NULL
    };

    if (argc == 1) {
        opt->displayUsage = 1;
        return;
    }
    opt->toPing = argv[1];
}

void
displayUsage()
{
    printf("Usage: ft_ping [-vh] [-s packetsize] [-t ttl] destination\n");
    printf("\t-v : Display packet errors\n");
    printf("\t-h : Display usage\n");
}