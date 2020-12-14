/* mpftpd.c - main, mpftpd */

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

#define QLEN 32 /* maximum connection queue length	*/

void reaper(int);
int ftpd(int listened_sock, int accepted_ctrl_sock);
int passiveTCP(const char *service, int qlen);
int mpftpd(const char *service)
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
            int return_code = ftpd(listened_sock, accepted_ctrl_sock);
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
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    char *service = SERVICE; /* service name or port number	*/

    switch (argc)
    {
    case 1:
        return mpftpd(service);
    case 2:
        return mpftpd(argv[1]);
    default:
        fprintf(stderr, "usage: mpftpd [port]\n");
        return 1;
    }
}

/*------------------------------------------------------------------------
 * ftpd - echo data until end of file
 *------------------------------------------------------------------------
 */
int ftpd(int listened_sock, int accepted_ctrl_sock)
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

        int fd;
        if (-1 == (fd = open(filename, O_RDONLY)))
        {
            char err_code = FOPEN_RETURNS_NULL;
            send(accepted_ctrl_sock, &err_code, sizeof(err_code), 0);
            break;
        }
        struct stat stat_buf;
        fstat(fd, &stat_buf);
        send(accepted_ctrl_sock, &stat_buf.st_size, sizeof(stat_buf.st_size), 0);

        int accepted_file_sock = accept(listened_sock, NULL, NULL);
        sendfile(accepted_file_sock, fd, NULL, stat_buf.st_size);
        close(accepted_file_sock);

        close(fd);

    } while (1);

    // while (cc = read(accepted_ctrl_sock, buf, sizeof buf)) {
    // 	if (cc < 0)
    // 		errexit("echo read: %s\n", strerror(errno));
    // 	if (write(accepted_ctrl_sock, buf, cc) < 0)
    // 		errexit("echo write: %s\n", strerror(errno));
    // }
    return 0;
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
