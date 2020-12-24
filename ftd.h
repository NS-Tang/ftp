#ifndef FTD_H
#define FTD_H

/**
 * 一个服务端
 * @param listened_file_sock 已被listen的文件socket
 * @param accepted_ctrl_sock 已被accepted的控制socket
 */
int ftd(int listened_file_sock, int accepted_ctrl_sock);
#endif