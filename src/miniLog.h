#ifndef _MINI_LOG_H_
#define _MINI_LOG_H_

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*Define log level.*/
#define INFO 1
#define WARN 2
#define ERROR 3
/*Define buff size*/
#define MINI_BUFFSIZE 256

/*Open the log file. If seccuss, return 0, else -1.*/
int openMiniLog(const char *logName);
/*Write content to log file, called after openMiniLog().*/
int writeMiniLog(int level, const char *format_str, ...);
/*Close the log file opened by openMiniLog().*/
int closeMiniLog(void);
#endif