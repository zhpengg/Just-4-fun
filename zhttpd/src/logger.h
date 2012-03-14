#ifndef __LOGGER__
#define __LOGGER__

#define LOG_ACCESS_FILE "log/zhttpd.access"
#define LOG_MSG_FILE "log/zhttpd.msg"

#include "global.h"

/* 用户访问记录 */
/* Format: IP [TIME] "URL"*/
void log_access(char *ip, char *url, int status);

/* 一般消息记录 */
/* Format: [PID] [TIME] [MSG]*/
void log_message(char *fmt, ...);

void log_init(http_setting_t *st);

#endif
