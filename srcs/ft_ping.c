#include "ft_ping.h"

static void
cleanEnv(t_env *e)
{
    if (e->resolved) {
        freeaddrinfo(e->resolved);
    }
    if (e->socket) {
        close(e->socket);
    }
}

static uint8_t
resolveAddrToPing(t_env *e)
{
    if (!(e->resolved = resolveAddr(e->opt.toPing, e->opt.verbose))) {
        printf("ft_ping: %s: Name or service not known\n", e->opt.toPing);
        return (1);
    }
    if (getValidIp(e->resolved, &e->dest.addrDest, e->opt.verbose)) {
        printf("ft_ping: No valid ip for name or service\n");
        cleanEnv(e);
        return (1);
    }
    if (!inet_ntop(AF_INET,
                   &((struct sockaddr_in *)e->dest.addrDest->ai_addr)->sin_addr,
                   e->dest.ip,
                   INET_ADDRSTRLEN)) {
        printf("ft_ping: Ip conversion failed\n");
        cleanEnv(e);
        return (1);
    }
    if (!e->opt.noLookup) {
        e->dest.dispFqdn = strcmp(e->dest.addrDest->ai_canonname, e->dest.ip);
    }
    if (e->dest.dispFqdn &&
        reverseResolveDest(
          e->dest.fqdn, NI_MAXHOST, e->dest.addrDest, e->opt.verbose)) {
        cleanEnv(e);
        return (1);
    }
    return (0);
}

static uint8_t
initEnv(t_env *e)
{
    if (resolveAddrToPing(e)) {
        return (1);
    }
    if ((e->socket = initSocket(&e->opt)) < 3) {
        cleanEnv(e);
        return (1);
    }
    e->packetSize =
      e->opt.icmpMsgSize + sizeof(struct icmphdr) + sizeof(struct iphdr);
    g_loopControl.loop = 1;
    signal(SIGINT, stopLoop);
    signal(SIGALRM, stopWait);
    return (0);
}

int
main(int32_t argc, char const **argv)
{
    t_env e = { -1, 0, NULL, { 0 }, { 0 } };
    struct timeval start;

    if (getuid()) {
        printf("ft_ping: not enough privilege, use sudo\n");
        displayUsage();
        return (0);
    }
    parseOptions(&e.opt, argc, argv);
    if (e.opt.displayUsage) {
        displayUsage();
        return (-1);
    }
    if (initEnv(&e)) {
        return (-1);
    }
    gettimeofday(&start, NULL);
    loop(&e, convertTime(&start));
    cleanEnv(&e);
    return (0);
}