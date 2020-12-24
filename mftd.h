#ifndef MFTD_H
#define MFTD_H
/**
 * 并发服务端
 * @param ctrl_service 控制socket所用端口
 * @param file_service 文件socket所用端口
 */
int mftd(const char *ctrl_service, const char *file_service);
#endif