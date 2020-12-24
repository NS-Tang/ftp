#include <limits.h>

#include <endian.h>
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
        // remote末尾之后的指针
        const char *end = (const char *)(&remote + 1);
        // 遍历recv，获取文件名
        for (char *iter = remote;;)
        {
            iter += recv(accepted_ctrl_sock, iter, end - iter, 0);
            if (iter[-1] == '\0')
            {
                if (iter != remote + 1)
                {
                    break;
                }
                else
                {
                    return 0;
                }
            }
            if (iter == end)
            {
                return -1;
            }
        }
        // 打开文件
        int remote_fd = open(remote, O_RDONLY);
        char ctrl_code;
        // 发送错误码，如果没错发送0
        if (-1 == remote_fd)
        {
            ctrl_code = errno;
            send(accepted_ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
            continue;
        }
        else
        {
            ctrl_code = 0;
            send(accepted_ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
        }

        // 获取文件大小
        struct stat file_stat;
        fstat(remote_fd, &file_stat);
        // 发送文件大小
        off_t st_size_be = htobe64(file_stat.st_size);
        send(accepted_ctrl_sock, &st_size_be, sizeof(st_size_be), 0);

        // 由文件fd直接向文件socket发送文件内容
        int accepted_file_sock = accept(listened_file_sock, NULL, NULL);
        sendfile(accepted_file_sock, remote_fd, NULL, file_stat.st_size);
        close(accepted_file_sock);

        close(remote_fd);

    } while (1);
}
