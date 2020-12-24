#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ftd.h"
#include "mftd.h"
#include "proto.h"

#define QLEN 32 /* maximum connection queue length	*/

static size_t THREAD_ARRAY_LEN = 1 << 3;

int passiveTCP(const char *service, int qlen);

typedef struct
{
    int listened_file_sock;
    int listened_ctrl_sock;
} FtdArg;

/**
 * 一个简单的循环服务端
 */
void *ftd_start_routine(void *args)
{
    const FtdArg *arg = (const FtdArg *)args;
    while (1)
    {
        int accepted_ctrl_sock = accept(arg->listened_ctrl_sock, NULL, NULL);
        if (accepted_ctrl_sock < 0)
        {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "accept: %s\n", strerror(errno));
            continue;
        }
        ftd(arg->listened_file_sock, accepted_ctrl_sock);
        close(accepted_ctrl_sock);
    }
}

int mftd(const char *ctrl_service, const char *file_service)
{

    FtdArg arg;

    // 监听文件和控制端口
    arg.listened_ctrl_sock = passiveTCP(ctrl_service, QLEN);
    arg.listened_file_sock = passiveTCP(file_service, QLEN);

    // 线程池
    pthread_t thread_array[THREAD_ARRAY_LEN];
    // 线程池末尾之后
    pthread_t *const end = (pthread_t *)(&thread_array + 1);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // 设置线程属性为detached
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // 创建固定数线程
    for (pthread_t *iter = thread_array; iter != end; ++iter)
    {
        pthread_create(iter, NULL, ftd_start_routine, &arg);
    }
    pthread_attr_destroy(&attr);
    // 等待进程终止信号
    while (1)
        ;
    return 0;
}