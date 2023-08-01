/*************************************************************************
    > File Name: log.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>

/**
 * @brief Not used, for learn 
 * 
 */

// 用于记录程序的行为
void logcmd(char *fmt, ...);

// 打开日志文件
int init_logfile(char *filename);

// 将程序运行日志记录到文件
int logfile(char *file, int line, char *msg);

#endif
