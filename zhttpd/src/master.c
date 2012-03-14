#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include "master.h"
#include "logger.h"
#include "worker.h"

int server_listen(http_setting_t *st)
{
	int fd, on = 1;
	struct sockaddr_in servaddr;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_message("socket: %s\n", strerror(errno));
		exit(-1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(st->hs_port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		log_message("setsockopt: %s\n", strerror(errno));
		exit(-1);
	}

	if (bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		log_message("bind: %s\n", strerror(errno));
		exit(-1);
	}

	if (listen(fd, st->hs_wqlen) < 0) {
		log_message("listen: %s\n", strerror(errno));
		exit(-1);
	}

	log_message("Server listening on port: %d\n", st->hs_port);

	return fd;
}

int server_init_worker(http_setting_t *st, int listfd)
{
	int i;
	pid_t pid;

	for (i = 0; i < st->hs_wknum; i++) {
		if ((pid = fork()) < 0) {
			log_message("fork error:%s\n", strerror(errno));
			continue;
		} else if (pid == 0) {
			/* 开始worker进程 */
			do_worker(listfd);
			return 0;
		} else {
			log_message("Master %u fork child %u\n", getpid(), pid);
		}
	}

	while ((pid = waitpid(-1, NULL, 0)) > 0) {
		log_message("child %u died\n", pid);
	}
	return 0;
}

void daemonize(http_setting_t *st)
{
	pid_t pid;
	int fd0, fd1, fd2;

	if ((pid = fork()) < 0) {
		log_message("%s\n", strerror(errno));
		exit(-1);
	} else if (pid > 0) {
		exit(0);
	}

	setsid();

	if ((pid = fork()) < 0) {
		log_message("fork2: %s\n", strerror(errno));
	} else if (pid > 0) {
		exit(0);
	}

	if(chdir(st->hs_docroot) < 0) {
		log_message("chdir: %s\n", strerror(errno));
		exit(-1);
	}

	close(0);
	close(1);
	close(2);
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
}

void sigchld_handler(int signo)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		log_message("Child %d died\n", pid);
	}
}
