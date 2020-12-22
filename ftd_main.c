#include "mftd.h"
#include "proto.h"

int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        return mftd(CTRL_SERVICE, FILE_SERVICE);
    case 3:
        return mftd(argv[1], argv[2]);
    default:
        fprintf(stderr, "usage: %s [control port] [local port]\n", argv[0]);
        return 1;
    }
}