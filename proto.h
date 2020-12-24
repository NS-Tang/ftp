#ifndef PROTO_H
#define PROTO_H
/**
 * @file 客户端和服务端的共用定义
 */

#include <stdio.h>

// 默认控制端口
static const char CTRL_SERVICE[] = "5141";
// 默认文件端口
static const char FILE_SERVICE[] = "5140";
// 文件长度
#define FILENAME_LEN FILENAME_MAX
// 文件名类型
typedef char Filename[FILENAME_LEN + 1];

#endif