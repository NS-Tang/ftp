/* mpftd.c - main, mpftd */

#define _USE_BSD
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

#define QLEN 32 /* maximum connection queue length	*/

void reaper(int);
int ftd(int listened_sock, int accepted_ctrl_sock);
int passiveTCP(const char *service, int qlen);
int mpftd(const char *service);

/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        return mpftd(SERVICE);
    case 2:
        return mpftd(argv[1]);
    default:
        fprintf(stderr, "usage: mpftd [port]\n");
        return 1;
    }
}

int mpftd(const char *service)
{
    int listened_sock = passiveTCP(service, QLEN); /* master server socket		*/

    (void)signal(SIGCHLD, reaper);

    while (1)
    {
        int accepted_ctrl_sock = accept(listened_sock, NULL, NULL); /* slave server socket		*/
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
            int return_code = ftd(listened_sock, accepted_ctrl_sock);
            (void)close(accepted_ctrl_sock);
            (void)close(listened_sock);
            return return_code;
        }
        default: /* parent */
            (void)close(accepted_ctrl_sock);
            break;
        case -1:
            (void)close(accepted_ctrl_sock);
            (void)close(listened_sock);
            fprintf("fork: %s\n", strerror(errno));
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
int ftd(int listened_sock, int accepted_ctrl_sock)
{
    do
    {
        char filename[FILENAME_LEN];
        const char *end = &filename + 1;
        for (char *iter = filename;; ++iter)
        {
            recv(accepted_ctrl_sock, iter, sizeof(*iter), 0);
            if (*iter == '\0')
            {
                break;
            }
            if (iter == end)
            {
                return -1;
            }
        }

        int file = open(filename, O_RDONLY);
        char ctrl_code = file >> (sizeof(file) - 1) * CHAR_BIT;
        send(accepted_ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
        if (-1 == file)
        {
            break;
        }
        struct stat file_stat;
        fstat(file, &file_stat);
        send(accepted_ctrl_sock, &file_stat.st_size, sizeof(file_stat.st_size), 0);

        int accepted_file_sock = accept(listened_sock, NULL, NULL);
        sendfile(accepted_file_sock, file, NULL, file_stat.st_size);
        close(accepted_file_sock);

        close(file);

    } while (1);
    return 0;
}
