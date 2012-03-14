#ifndef __GLOBAL__
#define __GLOBAL__

#include <linux/limits.h>
#include <sys/types.h>
#include <sys/socket.h>

struct http_settings{
	unsigned short hs_port;	/* server listening port */
	char hs_docroot[PATH_MAX];	/* document root */
	unsigned int hs_wqlen;	/* waiting queue length */
	unsigned int hs_wknum;	/* worker process number */
};

typedef struct http_settings http_setting_t;

/* 记录每一个连接信息 */
struct http_connection {
	int hc_connfd;
	struct sockaddr hc_addr;
};

typedef struct http_connection http_conn_t;

#define HTTP_TIMEOUT 30

#define SERVER_INFO "ZHTTPD 0.0.1"

http_setting_t hs;
#endif
