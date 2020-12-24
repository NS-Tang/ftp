#ifndef PROTO_H
#define PROTO_H

#include <stdio.h>

static const char CTRL_SERVICE[] = "5141";
static const char FILE_SERVICE[] = "5140";
#define FILENAME_LEN FILENAME_MAX
typedef char Filename[FILENAME_LEN + 1];

#endif