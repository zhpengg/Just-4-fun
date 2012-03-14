#ifndef __MASTER__
#define __MASTER__

#include "global.h"

int server_listen(http_setting_t *st);

/* 服务器初始化worker 进程 */
int server_init_worker(http_setting_t *st, int listfd);

void daemonize(http_setting_t *st);

void sigchld_handler(int signo);
#endif
