#include "ft_ping.h"

static void
cleanEnv(t_env *e)
{
    freeaddrinfo(e->dest);
}

int
main(int argc, char **argv)
{
    t_env e = { -1, NULL, NULL };

    if (argc != 2) {
        printf("Usage : %s destination\n", argv[0]);
        return (-1);
    }
    if (!(e.resolved = resolveAddr(argv[1]))) {
        printf("%s\n", "Error : resolving address");
        return (-1);
    }
    printf("%s\n", "Resolved IP : ");
    dbg_printListAddrInfo(e.resolved);
    if (getValidIp(e.resolved, &e.dest)) {
        printf("%s\n", "Error : no valid IP for domain");
        cleanEnv(&e);
        return (-1);
    }
    printf("%s\n", "Selected IP : ");
    dbg_printAddrInfo(e.dest);
    if ((e.socket = initSocket()) < 3) {
        printf("%s\n", "Error initializing socket : maybe you should sudo");
        cleanEnv(&e);
        return (-1);
    }
    cleanEnv(&e);
    return (0);
}