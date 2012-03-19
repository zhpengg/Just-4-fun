#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>

#include <sys/epoll.h>

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
    return ret;
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
    int lsnfd = open_socket(LSN_ADDRESS, LSN_PORT);
    if (lsnfd < 0) return;

    const int max_events = 1024;
    struct pollfd pfds[max_events], pfdsback[max_events];
    int i;
    for (i = 0; i < max_events; i++)
        pfdsback[i].fd = -1;

    pfds[lsnfd].fd = lsnfd;
    pfds[lsnfd].events = POLLIN;
    int totalfds = lsnfd+1;
    int timeout = 2000;
    for (;;) {
        int nfds = poll(pfds, totalfds, timeout);
        if (nfds == 0) {
            printf("time out\n");
            continue;
        } else if (nfds < 0) {
            perror("poll");
            return;
        }

        for (i = 0; i < max_events && nfds; i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == lsnfd) {
                    int acpfd = do_accept(lsnfd);
                    if (acpfd > 0) {
                        pfds[acpfd].fd = acpfd;
                        pfds[acpfd].events = POLLIN;
                    }
                    if (acpfd + 1 > totalfds)
                        totalfds = acpfd + 1;
                } else {
                    int ret = do_read(pfds[i].fd);
                    if (ret < 0) {
                        close(pfds[i].fd);
                        pfds[i].fd = -1;
                    }
                    if (i + 1 >= totalfds)
                        totalfds = i;
                }
                nfds--;
            }
        }
    }
}

void echo_epoll()
{
    int lsnfd = open_socket(LSN_ADDRESS, LSN_PORT);
    if (lsnfd < 0) return;

    int epfd = epoll_create(1024);
    if (epfd < 0) {
        perror("epoll_create");
        return;
    }

    const int max_events = 1024;
    struct epoll_event ev, events[max_events];
    ev.events = EPOLLIN;
    ev.data.fd = lsnfd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lsnfd, &ev);
    if (ret != 0) {
        perror("epoll_ctl");
        return;
    }

    int i, nfds = 0, timeout = 2000;
    for (;;) {
        nfds = epoll_wait(epfd, events, max_events, timeout); 
        if (nfds < 0) {
            perror("epoll wait");
            return;
        } else if (nfds == 0) {
            printf("timeout\n");
            continue;
        }
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == lsnfd) {
                // do accesp
                int acpfd = do_accept(lsnfd);
                if (acpfd > 0) {
                    ev.data.fd = acpfd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, acpfd, &ev);
                }
            } else {
                // do read
                int ret = do_read(events[i].data.fd);
                if (ret < 0) {
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                }
            }
        }
    }	
}
