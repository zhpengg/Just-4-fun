#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include "logger.h"
#include "parse_args.h"
#include "readline.h"
#include "parse_request.h"
#include "worker.h"
#include "global.h"
#include "file_access.h"

extern http_setting_t hs;

static void do_request(int connfd, http_request_t *request);

static void request_dump(http_request_t *req);

static int is_timeout = 0;

void worker_sigalarm_handler(int signo);

void do_worker(int listfd)
{
	struct sockaddr_in clitaddr = {0};
	size_t len = sizeof(clitaddr);
	int connfd;

	/* 安装keepalive超时处理函数 */
	struct sigaction act;
	act.sa_handler = worker_sigalarm_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		log_message("sigaction: %s\n", strerror(errno));
	}

AGAIN:	
	while ((connfd = accept(listfd, (struct sockaddr*)&clitaddr, &len)) > 0) {
		http_request_t request, *rptr;

		/* FIXME 先不管keepalive了 */
		/* 读取http请求 */
		bzero(&request, sizeof(request));
		rptr = parse_request(connfd, &request);
		memcpy(&request.hr_addr, &clitaddr, sizeof(clitaddr));

		/* 处理http request */
		do_request(connfd, &request);

		close(connfd);
	}
	if (connfd < 0) {
		log_message("accept %s\n", strerror(errno));
		close(connfd);
		goto AGAIN;
	}
}

static void request_dump(http_request_t *empty)
{
	log_message("Method: %d\n", empty->hr_method);
	log_message("uRI: %s\n", empty->hr_uri);
	log_message("Host: %s\n", empty->hr_host);
	log_message("Version: %d\n", empty->hr_version);
	log_message("User-agent: %s\n", empty->hr_ua);
	log_message("Keep-Alive: %d\n", empty->hr_keepalive);
}

void do_request(int connfd, http_request_t *request)
{
	extern http_setting_t hs;
	struct stat statbuff;
	char resheader[1024];

	if (test_and_log(request, &hs, &statbuff) == -1) {
		//gen_err_header(resheader, errno, &hs);
		log_message("No file");
	} else {
		gen_ok_header(resheader, &statbuff, request);
		write(connfd, resheader, strlen(resheader));
		send_file(connfd, request->hr_uri, &statbuff);
	}
}

void worker_sigalarm_handler(int signo)
{
	is_timeout = 1;
}
