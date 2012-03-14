#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "parse_args.h"
#include "master.h"
#include "server.h"
#include "logger.h"

extern http_setting_t hs;

int main (int argc, char **argv)
{
	int lisfd;

	/* 解析用户命令，初始化配置信息 */
	parse_args(argc, argv, &hs);
	log_init(&hs);

	/* 将主进程变成守护进程 */
	daemonize(&hs);
	log_message("daemon ok\n");

	/* 服务器打开监听端口 */
	lisfd = server_listen(&hs);
	log_message("listen socket:%d\n", lisfd);

	/* 初始化worker进程 */
	server_init_worker(&hs, lisfd);

	return 0;
}

