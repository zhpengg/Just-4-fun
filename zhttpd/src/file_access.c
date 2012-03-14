#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <linux/limits.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include "global.h"
#include "parse_args.h"
#include "logger.h"
#include "file_access.h"

static char *header = "HTTP/1.1 %d %s\r\nDate: %sContent-Type: %s\r\nContent-Length: %d\r\nServer: %s\r\n\r\n";

int test_and_log(http_request_t *req, http_setting_t *st, struct stat *statbuff)
{
	char addrbuff[INET_ADDRSTRLEN];
	char pathtmp[PATH_MAX];
	int tmperrno;
	struct sockaddr_in *clitaddr;

	clitaddr = (struct sockaddr_in*)&(req->hr_addr);
	sprintf(pathtmp, "%s%s", st->hs_docroot, req->hr_uri);
	inet_ntop(AF_INET, &(clitaddr->sin_addr), addrbuff, sizeof(addrbuff));

	if (stat(pathtmp, statbuff) == -1) {
		tmperrno = errno;
		log_access(addrbuff, req->hr_uri, 404);
		errno = tmperrno;
		return -1;
	} else {
		tmperrno = errno;
		log_access(addrbuff, req->hr_uri, 200);
		errno = tmperrno;
		return 0;
	}
}

char *gen_err_header(char *headerbuff, int errno, http_setting_t *st)
{
	time_t tt = time(NULL);
	struct stat statbuff;
	char errpage[PATH_MAX] = {0};

	switch (errno) {
		case ENOENT:
			sprintf(errpage, "%s%s", st->hs_docroot, ERR_404_PAGE);
			if (stat(errpage, &statbuff) == -1) {
				return NULL;
			}
			sprintf(headerbuff, header, 404, 
					"File Not Found",
					ctime(&tt),
					"text/html;charset=UTF-8",
					statbuff.st_size,
					SERVER_INFO);
			return headerbuff;
			break;
		case EACCES:
		default:
			sprintf(errpage, "%s%s", st->hs_docroot, ERR_403_PAGE);
			if (stat(errpage, &statbuff) == -1) {
				return NULL;
			}
			sprintf(headerbuff, header, 403, 
					"Forbidden",
					ctime(&tt),
					"text/html;charset=UTF-8",
					statbuff.st_size,
					SERVER_INFO);
			return headerbuff;
			break;
	}
}

char *gen_ok_header(char *headerbuff, struct stat *statbuff, http_request_t *req)
{
	char ctype[128];
	char *type;
	time_t tt;
	
	tt = time(NULL);
	type = rindex(req->hr_uri, '.');
	type += 1;

	if (type == NULL || strncasecmp(type, "htm", 3) == 0) {
		snprintf(ctype, 128, "text/html;charset=utf-8");
	} else if (strncasecmp(type, "js", 2) == 0) {
		snprintf(ctype, 128, "text/javascript;charset=utf-8");
	} else if (strncasecmp(type, "css", 3) == 0) {
		snprintf(ctype, 128, "text/css;charset=utf-8");
	} else if (strncasecmp(type, "gif", 3) == 0) {
		snprintf(ctype, 128, "image/gif");
	} else if (strncasecmp(type, "jpeg", 4) == 0) {
		snprintf(ctype, 128, "image/jpeg");
	} else if (strncasecmp(type, "jpg", 3) == 0) {
		snprintf(ctype, 128, "image/jpg");
	} else {
		snprintf(ctype, 128, "text/html;charset=utf-8");
	}

	sprintf(headerbuff, header, 200, 
			"OK",
			ctime(&tt),
			ctype,
			statbuff->st_size,
			SERVER_INFO);
	return headerbuff;
}

void send_file(int connfd, char *filepath, struct stat *statbuff)
{
	extern http_setting_t hs;

	char *fileaddr, tmppath[PATH_MAX];
	int ofd;

	sprintf(tmppath, "%s%s", hs.hs_docroot, filepath);
	if ((ofd = open(tmppath, O_RDONLY)) == -1) {
		log_message("open:%s\n", strerror(errno));
		return;
	}

	if (stat(tmppath, statbuff) == -1) {
		log_message("stat: %s\n", strerror(errno));
		return;
	}
	if ((fileaddr = mmap(NULL, statbuff->st_size, PROT_READ, MAP_PRIVATE, ofd, 0)) == MAP_FAILED) {
		log_message("mmap: %s\n", strerror(errno));
		return;
	}

	if (write(connfd, fileaddr, statbuff->st_size) < 0) {
		log_message("write: %s\n", strerror(errno));
		return;
	}	
}
