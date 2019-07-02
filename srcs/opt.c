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
        } else if (arg[i] == 'q') {
            opt->quiet = 1;
        } else if (arg[i] == 'n') {
            opt->noLookup = 1;
        } else if (arg[i] == 'D') {
            opt->printTs = 1;
        } else if (arg[i] == 'f') {
            opt->flood = 1;
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
    if (i == 0) {
        if (val < 0 || (uint64_t)val > MAX_PACKET_SIZE) {
            printf("ft_ping: invalid packet size: %d\n", val);
            opt->displayUsage = 1;
            return (1);
        }
        opt->icmpMsgSize = val;
    } else if (i == 1) {
        if (val < 0 || (uint64_t)val > MAX_TTL) {
            printf("ft_ping: invalid ttl value: %d\n", val);
            opt->displayUsage = 1;
            return (1);
        }
        opt->ttl = val;
    } else if (i == 2) {
        if (val < 0) {
            printf("ft_ping: bad wait time: %d\n", val);
            opt->displayUsage = 1;
            return (1);
        }
        opt->deadline = val * SEC_IN_US;
    }
    return (1);
}

static uint8_t
parseSingle(t_option *opt, char const *arg, char const *nextArgv)
{
    char const tab[NBR_OPTION][3] = { "-s", "-t", "-w", "-v", "-h",
                                      "-n", "-D", "-q", "-f" };

    for (uint64_t i = 0; i < NBR_OPTION; ++i) {
        if (!strcmp(arg, tab[i])) {
            if (i < 3) {
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
    *opt = (t_option){ 0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       TTL_DEFAULT,
                       { TIMEOUT_DEFAULT, 0 },
                       ICMP_MSG_SIZE_DEFAULT,
                       NULL };

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
    if (argv[argc - 1][0] == '-') {
        opt->displayUsage = 1;
        return;
    }
    opt->toPing = argv[argc - 1];
}

void
displayUsage()
{
    printf("Usage: ft_ping [-vhqnDf] [-s packetsize] [-t ttl] [-w deadline] "
           "destination\n");
    printf("\t-v : Display packet errors\n");
    printf("\t-h : Display usage\n");
    printf("\t-q : Quiet output\n");
    printf("\t-n : No name lookup for host address\n");
    printf("\t-D : Print timestamp before each line\n");
    printf("\t-f : Flood. No wait time between icmp request\n");
}