/**
  * 对文件描述符进行带缓冲的行读取 
  */
#ifndef __READLINE__
#define __READLINE__

#define FD_BUFFER_MAX 1024

struct fd_buffer {
	unsigned char fb_buff[FD_BUFFER_MAX];
	int fb_fd;
	int fb_cur;
	int fb_size;
};
typedef struct fd_buffer fd_buffer_t;

/* 初始化fd缓冲区 */
int fd_buffer_init(int fd, fd_buffer_t *buff);

/* 从缓冲区中读取一行 */
char *fd_buffer_read_line(void *line, int len, fd_buffer_t *buff);

#endif
