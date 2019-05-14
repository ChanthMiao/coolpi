#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "miniLog.h"
/*Shared file description.*/
int fd_MiniLog = -1;
/*Open the log file. If seccuss, return 0, else -1.*/
int openMiniLog(const char *logPath)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(logPath, &statbuff) < 0)
    {
        if (errno == ENOENT)
        {
            goto new_log;
        }
        return -1;
    }
    else if (statbuff.st_size > 31457280)
    {
        fd_MiniLog = open(logPath, O_WRONLY | O_TRUNC | O_APPEND);
        return 0;
    }
new_log:
    fd_MiniLog = open(logPath, O_WRONLY | O_APPEND | O_CREAT, 0664);
    return 0;
}
/*Write content to log file, called after openMiniLog().*/
int writeMiniLog(int level, const char *format_str, ...)
{
    if (fd_MiniLog < 0)
    {
        return -1;
    }
    int len;
    char buff[MINI_BUFFSIZE];
    time_t curr_time;
    struct tm *timeStruct = NULL;
    memset(buff, 0, MINI_BUFFSIZE);
    /*Write log time.*/
    time(&curr_time);
    timeStruct = localtime(&curr_time);
    len = strftime(buff, MINI_BUFFSIZE, "%F %T ", timeStruct);
    write(fd_MiniLog, buff, len);
    /*Write log level*/
    switch (level)
    {
    case INFO:
        strcpy(buff, "[INFO]:");
        len = 7;
        break;
    case WARN:
        strcpy(buff, "[WARN]:");
        len = 7;
        break;
    case ERROR:
        strcpy(buff, "[ERROR]:");
        len = 8;
        break;
    default:
        strcpy(buff, "[INFO]:");
        len = 7;
        break;
    }
    write(fd_MiniLog, buff, len);
    /*Write log content.*/
    va_list cust_args;
    va_start(cust_args, format_str);
    len = vsnprintf(buff, MINI_BUFFSIZE, format_str, cust_args);
    buff[len] = '\n';
    write(fd_MiniLog, buff, len + 1);
    return 0;
}
/*Close the log file opened by openMiniLog().*/
int closeMiniLog(void)
{
    if (fd_MiniLog != -1)
    {
        close(fd_MiniLog);
        fd_MiniLog = -1;
    }
}