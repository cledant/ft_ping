#include "ft_ping.h"

static uint8_t
parseMulti(t_option *opt, char const *arg, uint64_t len)
{
    if (arg[0] != '-') {
        opt->displayUsage = 1;
        return (0);
    }
    for (uint64_t i = 1; i < len; ++i) {
        if (arg[i] == 'h') {
            opt->displayUsage = 1;
        } else if (arg[i] == 'v') {
            opt->verbose = 1;
        }
    }
    return (0);
}

static uint8_t
parseInt(t_option *opt, char const *arg, uint64_t i)
{
    if (!arg) {
        opt->displayUsage = 1;
        return (0);
    }
    int32_t val = atoi(arg);
    if (i == 2) {
        if (val < 0 || (uint64_t)val > MAX_PACKET_SIZE) {
            printf("ft_ping: invalid packet size: %d\n", val);
            opt->displayUsage = 1;
            return (1);
        }
        opt->icmpMsgSize = val;
    } else if (i == 3) {
        if (val < 0 || (uint64_t)val > MAX_TTL) {
            printf("ft_ping: invalid ttl value: %d\n", val);
            opt->displayUsage = 1;
            return (1);
        }
        opt->ttl = val;
    }
    return (1);
}

static uint8_t
parseSingle(t_option *opt, char const *arg, char const *nextArgv)
{
    char const tab[NBR_OPTION][3] = { "-v", "-h", "-s", "-t" };

    for (uint64_t i = 0; i < NBR_OPTION; ++i) {
        if (!strcmp(arg, tab[i])) {
            if (i > 1) {
                return (parseInt(opt, nextArgv, i));
            } else {
                return (parseMulti(opt, arg, 2));
            }
        }
    }
    return (0);
}

static uint8_t
parseArg(t_option *opt, char const *argv, char const *nextArgv)
{
    uint64_t len = strlen(argv);

    if (len < 2) {
        opt->displayUsage = 1;
        return (0);
    } else if (len == 2) {
        return (parseSingle(opt, argv, nextArgv));
    } else {
        return (parseMulti(opt, argv, len));
    }
}

void
parseOptions(t_option *opt, int32_t argc, char const **argv)
{
    *opt = (t_option){
        0,   0,   TTL_DEFAULT, { TIMEOUT_DEFAULT, 0 }, ICMP_MSG_SIZE_DEFAULT,
        0.0, NULL
    };

    if (argc == 1) {
        opt->displayUsage = 1;
        return;
    }
    for (int32_t i = 1; i < (argc - 1); ++i) {
        char const *nextPtr = NULL;
        if ((i + 1) < (argc - 1)) {
            nextPtr = argv[i + 1];
        }
        i += parseArg(opt, argv[i], nextPtr);
    }
    opt->toPing = argv[argc - 1];
    opt->timeBetweenPacket = 1000.0;
}

void
displayUsage()
{
    printf("Usage: ft_ping [-vh] [-s packetsize] [-t ttl] destination\n");
    printf("\t-v : Display packet errors\n");
    printf("\t-h : Display usage\n");
}