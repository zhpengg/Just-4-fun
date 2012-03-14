#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "parse_request.h"
#include "readline.h"

http_request_t *parse_request(int fd, http_request_t *empty)
{
	fd_buffer_t buff;
	char line[1024], *method, *version, *uri;
	memset(empty, 0, sizeof(http_request_t));

	if (fd_buffer_init(fd, &buff) != 0) {
		return NULL;
	}

	while (strcmp(fd_buffer_read_line(line, 1024, &buff), "\r\n") != 0) {
		if ((strncasecmp(line, "GET", 3) == 0) || 
				strncasecmp(line, "POST", 3) == 0) {
			/* 使用GNU C 扩展的%as 读取不定长字符串 */
			sscanf(line, "%as %as %as", &method, &uri, &version);
			if (strcasecmp(method, "GET") == 0) {
				empty->hr_method = HTTP_GET;
			} else {
				empty->hr_method = HTTP_POST;
			}
			free(method);

			strncpy(empty->hr_uri, uri, sizeof(empty->hr_uri));
			free(uri);

			if (strcasecmp(version, "HTTP/1.1") == 0) {
				empty->hr_version = HTTP_1_1;
			} else {
				empty->hr_version = HTTP_1_0;
			}
			free(version);
			continue;
		}


		if (strncasecmp(line, "HOST", 4) == 0) {
			char *host, *title;
			sscanf(line, "%as %as", &title, &host);
			free(title);
			strncpy(empty->hr_host, host, sizeof(empty->hr_host));
			free(host);
			continue;
		}

		if (strncasecmp(line, "User-Agent", strlen("user-agent")) == 0) {
			strncpy(empty->hr_ua, line+strlen("User-Agent: "), sizeof(empty->hr_ua));
			continue;
		}

		if (strncasecmp(line, "Keep-Alive", strlen("Keep-Alive")) == 0) {
			char *title;
			int keepalive;
			sscanf(line, "%as %d", &title, &keepalive);
			free(title);
			empty->hr_keepalive = keepalive;
			continue;
		}
	}
	
	return empty;
}

int main_test (int argc, char **argv) {
	http_request_t empty;
	parse_request(STDIN_FILENO, &empty);

	printf("Method: %d\n", empty.hr_method);
	printf("uRI: %s\n", empty.hr_uri);
	printf("Version: %d\n", empty.hr_version);
	printf("User-agent: %s\n", empty.hr_ua);

	return 0;
}
