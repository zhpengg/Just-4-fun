#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>

#include "network.h"

int open_socket(const char *address, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        perror("socket");
        return -1;
    }

    int flag = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) != 0) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    int ret = inet_aton(address, &srvaddr.sin_addr);
    if (ret == 0) {
        perror("inet_aton");
        return -1;
    }

    ret = bind(sockfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
    if (ret != 0) {
        perror("bind");
        return -1;
    }

    ret = listen(sockfd, 511); // magic number from nginx
    if (ret != 0) {
        perror("listen");
        return -1;
    }
    return sockfd;
}

int make_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        perror("fcntl");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl set flags");
        return -1;
    }
    return 0;
}
