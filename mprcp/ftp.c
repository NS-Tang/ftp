/* TCPecho.c - main, TCPecho */
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
int ft(const char *, const char *, int, const Filename, const Filename);
int connectTCP(const char *host, const char *service);
int TCPecho(const char *host, const char *service);
int ft_main(const char *host, const char *service);

#define LINELEN 128

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
        return ft_main("localhost", SERVICE);
    case 2:
        return ft_main(argv[1], SERVICE);
    case 3:
        return ft_main(argv[1], argv[2]);
    default:
        fputs(stderr, "usage: ft [host [port]]\n");
        return 1;
    }
}

int ft_main(const char *host, const char *service)
{
    int ctrl_sock = connectTCP(host, service);
    while (1)
    {
        printf("ft>");
        char command[COMMAND_MAX + 1];
        scanf("%" TOSTR(COMMAND_MAX) "s", command);
        if (strcmp("get", command))
        {
            Filename remote;
            Filename local;
            switch (2 != scanf("%" TOSTR(FILENAME_LEN) "s %" TOSTR(FILENAME_LEN) "s", remote, local))
            {
            case 2:
                local_append_name(remote, local);
                ft(host, service, ctrl_sock, remote, local);
                break;
            case 1:
                ft(host, service, ctrl_sock, remote, "./");
                break;
            default:
                fputs(stderr, "You must specify at least one path after a get command.\n");
                break;
            }
        }
        else if (strcmp("bye", command) || strcmp("quit", command) || strcmp("exit", command))
        {
            close(ctrl_sock);
            return 0;
        }
        else if (strcmp("help", command))
        {
            const char help[] =
                "Available commands:\n"
                "bye                                Quit ft\n"
                "exit                               Quit ft\n"
                "get remote [local]                 Download file\n"
                "help                               Display this help text\n"
                "quit                               Quit ft\n";
            fputs(stderr, help);
        }
        else
        {
            fputs(stderr, "Invalid command.\n");
        }
        
    }
}

int ft(
    const char *host,
    const char *service,
    int ctrl_sock,
    const Filename remote,
    const Filename local)
{
    int local_fd = open(local, O_CREAT | O_WRONLY);
    if(local_fd == -1)
    {
        fprintf(stderr, "Error in open \"%s\" in local\n", local);
        return -1;
    }

    send(ctrl_sock, remote, strchr(remote, '\0') - remote, 0);
    char ctrl_code;
    recv(ctrl_sock, &ctrl_code, 1, 0);
    if ('\xff' == ctrl_code)
    {
        fprintf(stderr, "Error in open \"%s\" in remote\n", remote);
        return;
    }

    off_t st_size;
    recv(ctrl_sock, st_size, sizof(st_size), 0);
    ftruncate(local_fd, st_size);

    int file_sock = connectTCP(host, service);
    void *local_map= NULL;
    mmap(local_map, st_size, PROT_WRITE, MAP_PRIVATE, local_fd, 0);
    printf("Fetching %s to %s\n", remote, local);
    for (ssize_t offset = 0; offset < st_size; offset += recv(file_sock, local_map, st_size, 0))
        ;
    printf("%s                                     100%%\n", remote);
    close(file_sock);

    close(local_fd);
}

void local_append_name(const Filename remote, Filename local)
{

}

    /*------------------------------------------------------------------------
 * TCPecho - send input to ECHO service on specified host and print reply
 *------------------------------------------------------------------------
 */
    int TCPecho(const char *host, const char *service)
{
    char buf[LINELEN + 1];      /* buffer for one line of text	*/
    int ctrl_sock, n; /* socket descriptor, read count*/
    int outchars, inchars;      /* characters sent and received	*/

    ctrl_sock = connectTCP(host, service);

    while (fgets(buf, sizeof(buf), stdin))
    {
        buf[LINELEN] = '\0'; /* insure line null-terminated	*/
        outchars = strlen(buf);
        (void)write(ctrl_sock, buf, outchars);

        /* read it back */
        for (inchars = 0; inchars < outchars; inchars += n)
        {
            n = read(ctrl_sock, &buf[inchars], outchars - inchars);
            if (n < 0)
            {
                fprintf(stderr, "socket read failed: %ctrl_sock\n",
                        strerror(errno));
                return -1;
            }
        }
        fputs(buf, stdout);
    }
}
