
#define _USE_BSD
#include <sys/errno.h>
#include <sys/sendfile.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ftd.h"
#include "mftd.h"
#include "proto.h"


void reaper(int);
int passiveTCP(const char *service, int qlen);


int mftd(const char *ctrl_service, const char *file_service)
{
    int listened_ctrl_sock = passiveTCP(ctrl_service, QLEN); /* master server socket		*/
    int listened_file_sock = passiveTCP(file_service, QLEN);

    (void)signal(SIGCHLD, reaper);

    while (1)
    {
        int accepted_ctrl_sock = accept(listened_ctrl_sock, NULL, NULL); /* slave server socket		*/
        if (accepted_ctrl_sock < 0)
        {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "accept: %s\n", strerror(errno));
            return 1;
        }
        switch (fork())
        {
        case 0: /* child */
        {
            int return_code = ftd(listened_file_sock, accepted_ctrl_sock);
            (void)close(accepted_ctrl_sock);
            (void)close(listened_file_sock);
            (void)close(listened_ctrl_sock);
            return return_code;
        }
        default: /* parent */
            (void)close(accepted_ctrl_sock);
            break;
        case -1:
            (void)close(accepted_ctrl_sock);
            (void)close(listened_file_sock);
            (void)close(listened_ctrl_sock);
            fprintf(stderr, "fork: %s\n", strerror(errno));
            return 1;
        }
    }
}

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
void reaper(int sig)
{
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *)0) > 0)
        /* empty */;
}

/*------------------------------------------------------------------------
 * ftd - echo data until end of file
 *------------------------------------------------------------------------
 */
