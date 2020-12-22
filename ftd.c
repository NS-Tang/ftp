#include <limits.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ftd.h"
#include "mftd.h"
#include "proto.h"

int passiveTCP(const char *service, int qlen);

int ftd(int listened_file_sock, int accepted_ctrl_sock)
{
    do
    {
        Filename remote;
        const char *end = (const char *)(&remote + 1);
        for (char *iter = remote;; ++iter)
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

        int remote_fd = open(remote, O_RDONLY);
        char ctrl_code;
        if (-1 == remote_fd)
        {
            ctrl_code = errno;
            break;
        }
        else
        {
            ctrl_code = 0;
        }
        send(accepted_ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);

        struct stat file_stat;
        fstat(remote_fd, &file_stat);
        send(accepted_ctrl_sock, &file_stat.st_size, sizeof(file_stat.st_size), 0);

        int accepted_file_sock = accept(listened_file_sock, NULL, NULL);
        sendfile(accepted_file_sock, remote_fd, NULL, file_stat.st_size);
        close(accepted_file_sock);

        close(remote_fd);

    } while (1);
    return 0;
}
