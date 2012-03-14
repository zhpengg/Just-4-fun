#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "readline.h"

/* 从缓冲区中读取一个字符 */
static int fd_buffer_get_char(fd_buffer_t *buff);

/* 从fd直接读取数据 */
static int fd_buffer_read(int fd, void *buff, int len);

int fd_buffer_init(int fd, fd_buffer_t *buff)
{
	int n = 0;

	buff->fb_fd = fd;
	buff->fb_cur = 0;
	buff->fb_size = 0;

	if ((n = fd_buffer_read(fd, buff->fb_buff, FD_BUFFER_MAX)) < 0) {
		return -1;
	} else {
		buff->fb_size = n;
	}

#ifdef DEBUG
	printf("fd_buffer_init:\n");
	printf("fb_fd:%d\n", buff->fb_fd);
	printf("fb_size:%d\n", buff->fb_size);
	printf("fb_buff:%s\n", buff->fb_buff);
#endif
	
	return 0;
}

static int fd_buffer_get_char(fd_buffer_t *buff)
{
	int c, n;
	
again:
	if (buff->fb_size > 0) {
		c = buff->fb_buff[buff->fb_cur++];
		buff->fb_size--;
		return c;
	} else {
		if (( n = fd_buffer_read(buff->fb_fd, buff->fb_buff, sizeof(buff->fb_buff))) < 0) {
			return -1;
		}
		
		buff->fb_size = n;
		buff->fb_cur = 0;
		goto again;
	}
}

char *fd_buffer_read_line(void *line, int len, fd_buffer_t *buff)
{
	char *p = (char *)line;

	while (((*p++ = fd_buffer_get_char(buff)) != '\n') && --len)
		;
	*p++ = '\0';

	return (char *)line;
}

static int fd_buffer_read(int fd, void *buff, int len)
{
	int nleft = len, nread = 0, n = 0;

	
	while (nleft > 0) {
		if ((n = read(fd, buff+nread, nleft)) < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				return -1;
			}
		} else if (n == 0) {
			return nread;
		}
#ifdef DEBUG
		printf("fd_buffer_read:\n");
		printf("read:%s\n", (char*)buff);
#endif

		nleft -= n;
		nread += n;
		break;
	}

	return nread;
}
