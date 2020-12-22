#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "ftd.h"
#include "mftd.h"
#include "proto.h"

static size_t THREAD_ARRAY_LEN = 1 << 3;

int passiveTCP(const char *service, int qlen);

typedef struct
{
    int listened_file_sock;
    int listened_ctrl_sock;
} FtdArg;

void *ftd_start_routine(void *args)
{
    const FtdArg *arg = (const FtdArg *)args;
    while (1)
    {
        int accepted_ctrl_sock = accept(arg->listened_ctrl_sock, NULL, NULL); /* slave server socket		*/
        if (accepted_ctrl_sock < 0)
        {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "accept: %s\n", strerror(errno));
            return NULL;
        }
        ftd(arg->listened_file_sock, accepted_ctrl_sock);
    }
}

int mftd(const char *ctrl_service, const char *file_service)
{

    FtdArg arg;

    arg.listened_ctrl_sock = passiveTCP(ctrl_service, QLEN); /* master server socket		*/
    arg.listened_file_sock = passiveTCP(file_service, QLEN);

    pthread_t thread_array[THREAD_ARRAY_LEN];
    pthread_t *const end = (pthread_t *)(&thread_array + 1);

    for (pthread_t *iter = thread_array; iter != end; ++iter)
    {
        pthread_create(iter, NULL, ftd_start_routine, &arg);
    }
    wait(NULL);
    return 0;
}