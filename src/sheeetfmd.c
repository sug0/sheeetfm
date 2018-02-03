#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libmill.h>
#include "arg.h"

char *argv0;

static void usage(void)
{
    fprintf(stderr, "usage: %s -s <unix socket>\n", argv0);
    exit(1);
}

static coroutine void exec_rmt_cmd(unixsock as)
{
    char path[4096], cmd[4096], fullcmd[8192];

    memset(path, 0, sizeof(path));
    memset(cmd, 0, sizeof(cmd));
    memset(fullcmd, 0, sizeof(fullcmd));

    unixrecvuntil(as, path, sizeof(path), "\n", 1, -1);
    unixrecvuntil(as, cmd, sizeof(cmd), "\n", 1, -1);

    strtok(path, "\n");
    strtok(cmd, "\n");

    snprintf(fullcmd, sizeof(fullcmd),
             "clear && cd %s && %s", path, cmd);

    system(fullcmd);
    unixclose(as);
}

int main(int argc, char *argv[])
{
    unixsock ls;
    char *sock = NULL;

    argv0 = *argv;

    ARGBEGIN {
        case 's':
            sock = EARGF(usage());
            break;
        default:
            usage();
    } ARGEND;

    if (!sock)
        usage();
    
    remove(sock);
    ls = unixlisten(sock, 10);

    if (!ls) {
        perror("unixlisten");
        return 1;
    }

    chmod(sock, 0700);

    while (1) {
        unixsock as = unixaccept(ls, -1);

        if (!as)
            continue;

        go (exec_rmt_cmd(as));
    }

    unixclose(ls);

    return 0;
}
