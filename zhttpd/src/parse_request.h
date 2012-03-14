#ifndef __PARSE_REQUEST__
#define __PARSE_REQUEST__

#include <sys/types.h>
#include <sys/socket.h>
enum http_request_method{HTTP_GET, HTTP_POST, HTTP_HEAD};
typedef enum http_request_method request_method_t;

enum http_request_version{HTTP_1_1, HTTP_1_0};
typedef enum http_request_version request_version_t;

struct http_request {
	request_method_t hr_method;
	request_version_t hr_version;
	char hr_uri[1024];
	char hr_host[512];
	char hr_ua[1024]; /* user agent */
	int hr_keepalive;
	struct sockaddr hr_addr;
};

typedef struct http_request http_request_t;

http_request_t *parse_request(int fd, http_request_t *empty);

#endif
