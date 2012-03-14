#ifndef __SHARE_FDS__
#define __SHARE_FDS__
int send_fd(int fd, pid_t pid);

int recv_fd(pid_t pid);
#endif
