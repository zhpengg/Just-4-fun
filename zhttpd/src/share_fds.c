#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/limits.h>
#include "share_fds.h"
#include "logger.h"

int send_fd(int fd, pid_t pid)
{
	int unix_sock_fd = -1;
	struct sockaddr_un unix_socket_name = {0};
	char path[PATH_MAX] = {0};
	
	/* open unix socket */
	unix_socket_name.sun_family = AF_UNIX;
	snprintf(path, PATH_MAX, "/tmp/unix_socket_%u", pid);
	strncpy(unix_socket_name.sun_path, path, PATH_MAX);
	unix_sock_fd = socket(PF_UNIX, SOCK_DGRAM, 0);

	struct msghdr msg;
	char ccmsg[CMSG_SPACE(sizeof(fd))];
	struct cmsghdr *cmsg;
	struct iovec vec;

	char *str = "x";
	int rv;
	
	msg.msg_name = (struct sockaddr*)&unix_socket_name;
	msg.msg_namelen = sizeof(unix_socket_name);

	vec.iov_base = str;
	vec.iov_len = 1;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;

	msg.msg_control = ccmsg;
	msg.msg_controllen = sizeof(ccmsg);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	*(int*)CMSG_DATA(cmsg) = fd;
	msg.msg_controllen = cmsg->cmsg_len;

	msg.msg_flags = 0;

	rv = (sendmsg(unix_sock_fd, &msg, 0) != -1);
	if (rv) close(fd);
	return rv;
}

int recv_fd(pid_t pid)
{
	int fd;
	struct sockaddr_un unix_socket_name = {0};
	char path[PATH_MAX] = {0};
	snprintf(path, PATH_MAX, "/tmp/unix_socket_%u", pid);
	unix_socket_name.sun_family = AF_UNIX;
	strncpy(unix_socket_name.sun_path, path, PATH_MAX);
	
	if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
		log_message("unix socket: %s\n", strerror(errno));
		return -1;
	}
	if (bind(fd, (struct sockaddr*)&unix_socket_name, sizeof(unix_socket_name))) {
		close(fd);
		return -1;
	}

	/* reveive file descriptors */
	struct msghdr msg;
	struct iovec iov;
	char buff[1];
	int rv;
	int connfd = -1;
	char ccmsg[CMSG_SPACE(sizeof(connfd))];
	struct cmsghdr *cmsg;

	iov.iov_base = buff;
	iov.iov_len = 1;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = ccmsg;
	msg.msg_controllen = sizeof(ccmsg);

	rv = recvmsg(fd, &msg, 0);
	if (rv == -1) {
		log_message("recv fd failed: %s\n", strerror(errno));
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg->cmsg_type == SCM_RIGHTS) {
		log_message("recv fd, got unkown controll msg\n");
		return -1;
	}

	return *(int*)CMSG_DATA(cmsg);
}
