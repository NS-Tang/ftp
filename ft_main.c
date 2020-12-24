#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

void local_append_name(const Filename remote, Filename local);
void ft_once(const char *, const char *, const char *, int, const Filename, const Filename);
int connectTCP(const char *host, const char *service);
int TCPecho(const char *host, const char *service);
int ft(const char *, const char *, const char *);

#define COMMAND_MAX 4
#define _TOSTR(LEN) #LEN
#define TOSTR(LEN) _TOSTR(LEN)

/*------------------------------------------------------------------------
 * main - TCP client for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        return ft("localhost", CTRL_SERVICE, FILE_SERVICE);
    case 2:
        return ft(argv[1], CTRL_SERVICE, FILE_SERVICE);
    case 4:
        return ft(argv[1], argv[2], argv[3]);
    default:
        fprintf(stderr, "usage: %s [host [control port] [file port]]\n", argv[0]);
        return 1;
    }
}

int ft(const char *host, const char *ctrl_service, const char *file_service)
{
    int ctrl_sock = connectTCP(host, ctrl_service);
    while (1)
    {
        printf("ft> ");
        char command[COMMAND_MAX + 1];
        // 输入命令，指定宽度上限为COMMAND_MAX
        scanf("%" TOSTR(COMMAND_MAX) "s", command);
        // 输入的第一个词是"get"
        if (!strcmp("get", command))
        {
            Filename remote;
            Filename local;
            // 输入两个文件名
            switch (scanf("%" TOSTR(FILENAME_LEN) "s %" TOSTR(FILENAME_LEN) "s", remote, local))
            {
            case 2:
                // 空函数，本用与连接文件名和路径，可以不实现
                local_append_name(remote, local);
                // 调用一次ft
                ft_once(host, ctrl_service, file_service, ctrl_sock, remote, local);
                break;
            // case 1:
            //     ft_once(host, ctrl_service, file_service, ctrl_sock, remote, "./");
            //     break;
            default:
                fputs("You must specify two path after a get command.\n", stderr);
                break;
            }
        }
        // 退出ft
        else if (!(strcmp("bye", command) && strcmp("quit", command) && strcmp("exit", command)))
        {
            send(ctrl_sock, "", 1, 0);
            close(ctrl_sock);
            break;
        }
        // 显示帮助
        else if (!strcmp("help", command))
        {
            const char help[] =
                "Available commands:\n"
                "bye                                Quit ft\n"
                "exit                               Quit ft\n"
                "get remote [local]                 Download file\n"
                "help                               Display this help text\n"
                "quit                               Quit ft\n";
            fputs(help, stderr);
        }
        // 错误命令
        else
        {
            fputs("Invalid command.\n", stderr);
        }
        // 万能清空缓冲区语句
        int quit;
        while (quit = getchar() != '\n' && quit != EOF)
            ;
    }
    return 0;
}

/** 
 * 做一次文件传输
 * @param host         指定本地文件名
 * @param ctrl_service 控制端口
 * @param file_service 文件端口
 * @param ctrl_sock    控制socket
 * @param remote       要访问服务端的文件名
 * @param local        要存储客户端的文件名
 */
void ft_once(
    const char *host,
    const char *ctrl_service,
    const char *file_service,
    int ctrl_sock,
    const Filename remote,
    const Filename local)
{

    // 发送文件名，节省流量不发送字符串末尾之后的部分
    send(ctrl_sock, remote, strchr(remote, '\0') - remote + 1, 0);
    char ctrl_code;
    // 接收打开文件错误码
    recv(ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
    if (ctrl_code)
    {
        fprintf(stderr, "Error in open \"%s\" in remote: %s\n", remote, strerror(ctrl_code));
        return;
    }
    // 创建/打开文件
    int local_fd = open(local, O_CREAT | O_RDWR, 0664);
    if (local_fd == -1)
    {
        fprintf(stderr, "Error in open \"%s\" in local: %s\n", local, strerror(errno));
        return;
    }
    // 接收文件大小
    off_t st_size_be;
    recv(ctrl_sock, &st_size_be, sizeof(st_size_be), MSG_WAITALL);
    off_t st_size = be64toh(st_size_be);
    // 改变文件大小
    ftruncate(local_fd, st_size);
    // 建立文件与内存的映射
    char *const local_map = mmap(NULL, st_size, PROT_WRITE, MAP_SHARED, local_fd, 0);
    if (local_map == MAP_FAILED)
    {
        close(local_fd);
        fprintf(stderr, "Error in mmap to %s: %s\n", local, strerror(errno));
        return;
    }
    // 连接文件socket
    int file_sock = connectTCP(host, file_service);
    printf("Fetching %s to %s\n", remote, local);
    // 接收文件，写入映射内存
    char *const end = local_map + st_size;
    for (char *iter = local_map; iter < end;)
    {
        iter += recv(file_sock, iter, end - iter, 0);
    }
    printf("%s                                     100%%\n", remote);
    // 释放资源
    munmap(local_map, st_size);
    close(file_sock);

    close(local_fd);
}
/**
 * 未实现
 */
void local_append_name(const Filename remote, Filename local)
{
    // 可以在这里实现将remote中的文件名连接到local中的路径上
    // 若不实现则local必须指定完整文件名
}
