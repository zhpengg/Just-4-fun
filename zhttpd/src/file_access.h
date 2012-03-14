#ifndef __FIEL_ACCESS__
#define __FILE_ACCESS__

#include "parse_args.h"
#include "global.h"
#include "libgen.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "parse_request.h"

#define ERR_404_PAGE "/static/404.html"
#define ERR_403_PAGE "/static/403.html"

int test_and_log(http_request_t *req, http_setting_t *st, struct stat *statbuff);

char *gen_err_header(char *headerbuff, int errno, http_setting_t *st);

char *gen_ok_header(char *headerbuff, struct stat *statbuff, http_request_t *req);

void send_file(int connfd, char *filepath, struct stat *statbuff);
#endif
