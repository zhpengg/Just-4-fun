#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "srvmode.h"
#include "network.h"

#define LSN_ADDRESS "127.0.0.1"
#define LSN_PORT 12321

int do_accept(int fd)
{
	struct sockaddr_in cliaddr;
	socklen_t addrlen = sizeof(cliaddr);

	int accpfd = accept(fd, (struct sockaddr*)&cliaddr, &addrlen);
	if (accpfd < 0) {
		return -1;
	}

	printf("accept new connection from: %s:%d\n", 
			inet_ntoa(cliaddr.sin_addr), 
			ntohs(cliaddr.sin_port));
	return accpfd;
}

int do_read(int fd)
{
	int ret = 0;
	char buf[1024];
	if ((ret = read(fd, buf, sizeof(buf))) > 0) {
		// TODO loop read
		write(fd, buf, ret);
	} else {
		return -1;
	}
	return 0;
}

void echo_select()
{
    int lsnfd = open_socket(LSN_ADDRESS, LSN_PORT);
    if (lsnfd < 0) {
        return;
    }

	fd_set rdset, backset;
	FD_ZERO(&rdset);
	FD_SET(lsnfd, &rdset);
	backset = rdset;

	struct timeval timeout = {2, 0};
	int ready, i, maxfd = lsnfd + 1;
	while ((ready = select(maxfd, &rdset, NULL, NULL, &timeout)) >= 0) {
		if (ready == 0) {
			printf("timeout\n");
		} else {
			for (i = 0; i < maxfd && ready; i++) {
				if (FD_ISSET(i, &rdset)) {
					if (i == lsnfd) {
						// do accept
						int acpfd = do_accept(lsnfd);
						if (acpfd > 0) {
							FD_SET(acpfd, &backset);
							if (acpfd + 1 > maxfd)
								maxfd = acpfd + 1;
						}
					} else {
						// do read
						int ret = do_read(i);
						if (ret < 0) {
							FD_CLR(i, &backset);
						}
					}
					ready--;
				}
			}
		}
		rdset = backset;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
	}
}

void echo_poll()
{

}

void echo_epoll()
{

}
