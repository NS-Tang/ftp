
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

#define QLEN 32 /* maximum connection queue length	*/

void reaper(int);
int passiveTCP(const char *service, int qlen);

int mftd(const char *ctrl_service, const char *file_service)
{
    // 监听文件和控制两个端口
    int listened_ctrl_sock = passiveTCP(ctrl_service, QLEN);
    int listened_file_sock = passiveTCP(file_service, QLEN);
    // 注册信号处理函数
    (void)signal(SIGCHLD, reaper);

    while (1)
    {
        // accept控制socket
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
            // 调用一个客户端
            int return_code = ftd(listened_file_sock, accepted_ctrl_sock);
            (void)close(accepted_ctrl_sock);
            (void)close(listened_file_sock);
            (void)close(listened_ctrl_sock);
            return return_code;
        }
        default: /* parent */
            // 继续循环创造进程
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