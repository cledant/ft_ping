#include "ft_ping.h"

static void
cleanEnv(t_env *e)
{
    if (e->resolved) {
        freeaddrinfo(e->dest);
    }
    if (e->socket) {
        close(e->socket);
    }
}

static uint8_t
initEnv(t_env *e)
{
    if (!(e->resolved = resolveAddr(e->opt.toPing))) {
        printf("ft_ping : %s : Name or service not known", e->opt.toPing);
        return (-1);
    }
    if (getValidIp(e->resolved, &e->dest)) {
        printf("ft_ping : no valid IP for name or service");
        cleanEnv(e);
        return (-1);
    }
    if ((e->socket = initSocket(&e->opt)) < 3) {
        cleanEnv(e);
        return (-1);
    }
    e->packetSize = e->opt.icmpMsgSize + sizeof(struct icmphdr);
    memset(&g_loopControl, 1, sizeof(t_pingStat));
    signal(SIGINT, stopLoop);
    signal(SIGALRM, stopWait);
    return (0);
}

int
main(int argc, char const **argv)
{
    t_env e = { -1, NULL, NULL, { 0 }, 0 };

    parseOptions(&e.opt, argc, argv);
    if (e.opt.displayUsage) {
        displayUsage();
        return (-1);
    }
    if (initEnv(&e)) {
        return (-1);
    }
    loop(&e);
    cleanEnv(&e);
    return (0);
}