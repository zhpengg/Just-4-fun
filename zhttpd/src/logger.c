#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <libgen.h>
#include <time.h>
#include "logger.h"

static char log_access_file[PATH_MAX] = {0};
static char log_msg_file[PATH_MAX] = {0};

static void write_log(char *path, char *buff, size_t len)
{
	FILE *logfile;

	if ((logfile = fopen(path, "a+")) == NULL) {
		fprintf(stderr, "%s\n", path);
		perror("fopen");
	}

	fwrite(buff, sizeof(char), len, logfile);
	fclose(logfile);
}

void log_access(char *ip, char *url, int status)
{
	char buff[1024] = {0};
	time_t now = time(NULL);
	char *tt = ctime(&now);

	tt[strlen(tt)-2] = '\0';
	snprintf(buff, sizeof(buff), "%s [%s] \"%s\" [%d]\n", ip, tt, url, status);

	write_log(log_access_file, buff, strlen(buff));
}

void log_message(char *fmt, ...)
{
	char buff[1024] = {0};
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buff, fmt, ap);
	va_end(ap);

	write_log(log_msg_file, buff, strlen(buff));
}

void log_init(http_setting_t *st)
{
	char pathtmp[PATH_MAX];
	strncpy(pathtmp, st->hs_docroot, PATH_MAX);
	dirname(pathtmp);
	snprintf(log_access_file, PATH_MAX, "%s/log/zhttpd.access", pathtmp);
	snprintf(log_msg_file, PATH_MAX, "%s/log/zhttpd.msg", pathtmp);
}
