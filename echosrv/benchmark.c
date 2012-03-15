#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SRV_ADDRESS "127.0.0.1"
#define SRV_PORT 12321
#define LINE_MAX 1024

static inline void err_handle(const char *str)
{
	perror(str);
	exit(1);
}

void *echo(void *arg)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		err_handle("socket");
	struct sockaddr_in srvaddr;
	memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(SRV_PORT);
	int ret = inet_aton(SRV_ADDRESS, &srvaddr.sin_addr);
	if (ret == 0)
		err_handle("inet_ntoa");
	ret = connect(sockfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
	if (ret != 0)
		err_handle("connect");

	int count = *(int*)arg;
	char line[LINE_MAX] = "hellaksjdajlkdajkldajsklda;dladjaldadklsjaldj";
	size_t len = strlen(line);

	while (count--) {
		ret = write(sockfd, line, len);
		if (ret != len) {
			printf("write error after %d times write.\n", count);
			exit(-1);
		}
		ret = read(sockfd, line, sizeof(line));
		if (ret <= 0) {
			printf("read error, after %d times read\n", count);
			exit(-1);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 3) return -1;
	int thd_cnt = atoi(argv[1]);
	int loop_cnt = atoi(argv[2]);

	pthread_t *thd = (pthread_t *)malloc(thd_cnt * sizeof(pthread_t));
	if (thd == NULL)
		err_handle("malloc");
	int i;
	for (i = 0; i < thd_cnt; i++) {
		pthread_create(thd+i, NULL, echo, &loop_cnt);
	}

	for (i = 0; i < thd_cnt; i++) {
		pthread_join(thd[i], NULL);
	}

	printf("Done, %d threads, %d loops/thread\n", thd_cnt, loop_cnt);
	return 0;
}
