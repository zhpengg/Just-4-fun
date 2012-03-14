/**
  * 解析命令行参数，保存到配置信息中
  */
#ifndef __PARSE_ARGS__
#define __PARSE_ARGS__

#include "global.h"

int parse_args(int argc, char **argv, http_setting_t *hs);

#endif
