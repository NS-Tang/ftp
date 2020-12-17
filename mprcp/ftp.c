/* TCPecho.c - main, TCPecho */
#include <sys/socket.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

int connectTCP(const char *host, const char *service);
int TCPecho(const char *host, const char *service);
int ft_main(const char *host, const char *service);

#define LINELEN 128

#define COMMAND_MAX 4
#define _TOSTR(LEN) #LEN
#define TOSTR(LEN) _TOSTR(LEN)

void ft(const char *, const char *, int, const char[FILENAME_LEN + 1], const char[FILENAME_LEN + 1]);

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
            char filename[FILENAME_LEN + 1];
            char lfilename[FILENAME_LEN + 1];
            if (2 != scanf("%" TOSTR(FILENAME_LEN) "s %" TOSTR(FILENAME_LEN) "s", filename, lfilename))
            {
                fputs(stderr, "usage: get [remote file (path and )name] [local file (path and )name]\n");
            }
            ft(host, service, ctrl_sock, filename, lfilename);
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

void ft(
    const char *host,
    const char *service,
    int ctrl_sock,
    const char filename[FILENAME_LEN + 1],
    const char lfilename[FILENAME_LEN + 1])
{
    send(ctrl_sock, filename, strchr(filename, '\0'), 0);
    char ctrl_code;
    recv(ctrl_sock, &ctrl_code, sizeof(ctrl_code), 0);
    if ('\377' == ctrl_code)
    {
        fprintf(stderr, "Error in open the file \"%s\" at server", filename);
        return;
    }
    int file_sock = connectTCP(host, service);
    
    close(file_sock);
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
