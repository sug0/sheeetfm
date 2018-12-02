#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "arg.h"

#define die(MSG)  do {error = MSG; exit(1);} while (0)

int connected = 0, lfd;
char *argv0, *error = NULL;

static void usage(void)
{
    fprintf(stderr, "usage: %s -s <unix socket>\n", argv0);
    exit(1);
}

static void exit_sig(int signo)
{
    (void)signo;

    printf("\nQuit %s (y/n)? ", argv0);

    char ch = getchar();

    /* consume newline */
    getchar();

    if (ch == 'y')
        exit(0);
}

static void install_signals(void)
{
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
        die("failed to install SIGCHLD handler");

    if (signal(SIGQUIT, exit_sig) == SIG_ERR)
        die("failed to install SIGQUIT handler");

    if (signal(SIGINT, exit_sig) == SIG_ERR)
        die("failed to install SIGINT handler");
}

static void cleanup(void)
{
    if (connected)
        close(lfd);

    if (error)
        fprintf(stderr, "%s: %s\n", argv0, error);
}

static void connect_sock(const char *sock)
{
    struct sockaddr_un addr;

    remove(sock);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock, sizeof(addr.sun_path));

    lfd = socket(PF_UNIX, SOCK_STREAM, 0);

    if (lfd < 0)
        die("failed to create new unix domain socket");

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) != 0)
        die("failed to bind socket");

    if (listen(lfd, 5) != 0)
        die("failed to listen on socket");

    chmod(sock, 0700);
    connected = 1;
}

static void exec_rmt_cmd(int afd)
{
    char fullcmd[4096], path[4096], *cmd = path;

    memset(path, 0, sizeof(path));

    if (read(afd, path, sizeof(path)) < 0)
        return;

    if (!strsep(&cmd, "\n"))
        return;

    strtok(cmd, "\n");

    snprintf(fullcmd, sizeof(fullcmd),
             "clear && cd %s && %s", path, cmd);

    system(fullcmd);

    close(afd);
    exit(0);
}

int main(int argc, char *argv[])
{
    int afd;
    pid_t pid;
    char *sock = NULL;

    ARGBEGIN {
        case 's':
            sock = EARGF(usage());
            break;
        default:
            usage();
    } ARGEND;

    if (!sock)
        usage();

    atexit(cleanup);
    install_signals();
    connect_sock(sock);

    while (1) {
        afd = accept(lfd, NULL, NULL);

        if (afd < 0)
            continue;

        pid = fork();

        if (pid == 0)
            exec_rmt_cmd(afd);
    }

    return 0;
}
