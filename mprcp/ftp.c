/* TCPecho.c - main, TCPecho */
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
#define STR_FORMAT_WITH_LEN(LEN) "%"#LEN"s"

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
        fprintf(stderr, "usage: ft [host [port]]\n");
        return 1;
    }
}

int ft_main(const char *host, const char *service)
{
    while (1)
    {
        printf("ft>");
        char command[COMMAND_MAX + 1];
        scanf(COMMAND_FORMAT, command);
        if (strcmp("get", command))
        {
			char filename[FILENAME_LEN];

        }
        else if (strcmp("bye", command) || strcmp("quit", command))
        {
            return 0;
        }
        else
        {
            fprintf(stderr, "usage: get [path]          Download file\n");
            fprintf(stderr, "       bye                 Quit ft\n");
            fprintf(stderr, "       quit                Quit ft\n");
        }
    }
}

/*------------------------------------------------------------------------
 * TCPecho - send input to ECHO service on specified host and print reply
 *------------------------------------------------------------------------
 */
int TCPecho(const char *host, const char *service)
{
    char buf[LINELEN + 1]; /* buffer for one line of text	*/
    int connected_ctrl_sock, n;              /* socket descriptor, read count*/
    int outchars, inchars; /* characters sent and received	*/

    connected_ctrl_sock = connectTCP(host, service);

    while (fgets(buf, sizeof(buf), stdin))
    {
        buf[LINELEN] = '\0'; /* insure line null-terminated	*/
        outchars = strlen(buf);
        (void)write(connected_ctrl_sock, buf, outchars);

        /* read it back */
        for (inchars = 0; inchars < outchars; inchars += n)
        {
            n = read(connected_ctrl_sock, &buf[inchars], outchars - inchars);
            if (n < 0)
            {
                fprintf(stderr, "socket read failed: %connected_ctrl_sock\n",
                        strerror(errno));
                return -1;
            }
        }
        fputs(buf, stdout);
    }
}
