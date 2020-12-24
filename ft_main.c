/* TCPecho.c - main, TCPecho */
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
void ft(const char *, const char *, const char *, int, const Filename, const Filename);
int connectTCP(const char *host, const char *service);
int TCPecho(const char *host, const char *service);
int ft_main(const char *, const char *, const char *);

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
        return ft_main("localhost", CTRL_SERVICE, FILE_SERVICE);
    case 2:
        return ft_main(argv[1], CTRL_SERVICE, FILE_SERVICE);
    case 4:
        return ft_main(argv[1], argv[2], argv[3]);
    default:
        fprintf(stderr, "usage: %s [host [control port] [file port]]\n", argv[0]);
        return 1;
    }
}

int ft_main(const char *host, const char *ctrl_service, const char *file_service)
{
    int ctrl_sock = connectTCP(host, ctrl_service);
    while (1)
    {
        printf("ft> ");
        char command[COMMAND_MAX + 1];
        int quit;
        scanf("%" TOSTR(COMMAND_MAX) "s", command);
        if (!strcmp("get", command))
        {
            Filename remote;
            Filename local;
            switch (scanf("%" TOSTR(FILENAME_LEN) "s %" TOSTR(FILENAME_LEN) "s", remote, local))
            {
            case 2:
                local_append_name(remote, local);
                ft(host, ctrl_service, file_service, ctrl_sock, remote, local);
                break;
            case 1:
                ft(host, ctrl_service, file_service, ctrl_sock, remote, "./");
                break;
            default:
                fputs("You must specify at least one path after a get command.\n", stderr);
                break;
            }
        }
        else if (!(strcmp("bye", command) && strcmp("quit", command) && strcmp("exit", command)))
        {
            send(ctrl_sock, "", 1, 0);
            close(ctrl_sock);
            return 0;
        }
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
        else
        {
            fputs("Invalid command.\n", stderr);
        }
        while (quit = getchar() != '\n' && quit != EOF)
            ;
    }
}

void ft(
    const char *host,
    const char *ctrl_service,
    const char *file_service,
    int ctrl_sock,
    const Filename remote,
    const Filename local)
{

    send(ctrl_sock, remote, strchr(remote, '\0') - remote + 1, 0);
    char ctrl_code;
    recv(ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
    if (ctrl_code)
    {
        fprintf(stderr, "Error in open \"%s\" in remote: %s\n", remote, strerror(ctrl_code));
        return;
    }

    int local_fd = open(local, O_CREAT | O_RDWR, 0664);
    if (local_fd == -1)
    {
        fprintf(stderr, "Error in open \"%s\" in local: %s\n", local, strerror(errno));
        return;
    }
    off_t st_size_be;
    recv(ctrl_sock, &st_size_be, sizeof(st_size_be), MSG_WAITALL);
    off_t st_size = be64toh(st_size_be);

    ftruncate(local_fd, st_size);

    char *const local_map = mmap(NULL, st_size, PROT_WRITE, MAP_SHARED, local_fd, 0);
    if (local_map == MAP_FAILED)
    {
        close(local_fd);
        fprintf(stderr, "Error in mmap to %s: %s\n", local, strerror(errno));
        return;
    }

    int file_sock = connectTCP(host, file_service);
    printf("Fetching %s to %s\n", remote, local);
    char *const end = local_map + st_size;
    for (char *iter = local_map; iter < end;)
    {
        iter += recv(file_sock, iter, end - iter, 0);
    }
    printf("%s                                     100%%\n", remote);
    munmap(local_map, st_size);
    close(file_sock);

    close(local_fd);
}

void local_append_name(const Filename remote, Filename local)
{
}
