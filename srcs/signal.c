#include "ft_ping.h"

void
stopLoop(int32_t sig)
{
    (void)sig;
    g_loopControl.loop = 0;
    g_loopControl.wait = 0;
}

void
stopWait(int32_t sig)
{
    (void)sig;
    g_loopControl.wait = 0;
    signal(SIGALRM, stopWait);
}