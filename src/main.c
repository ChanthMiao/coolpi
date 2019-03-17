
#define _DEFAULT_SOURCE
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#define VERSION "v1.0.0"
#define NULL_DEVICE "/dev/null"
#define OPT_V 1
#define OPT_H 1 << 1
#define OPT_C 1 << 2
#define OPT_ERR 1 << 3
/*Define log file path.*/
#define LOG_FILE "/var/log/coolpi.log"

extern int fd_MiniLog;
extern const char *conf_file;
static int opt_index = 0;
static struct option cool_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {"configure", required_argument, NULL, 'c'},
    {0, 0, 0, 0}};

/*Offer feedback fuctions.*/
void showBaseInfo(const char *);

int main(int argc, char *argv[])
{
    /*Process the args.*/
    int opt, detected = 0;
    while ((opt = getopt_long(argc, argv, "vc:h", cool_options, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'v':
            detected |= OPT_V;
            break;
        case 'h':
            detected |= OPT_H;
            break;
        case 'c':
            detected |= OPT_C;
            conf_file = optarg;
            break;
        case '?':
            detected = OPT_ERR;
        default:
            detected |= 0;
        }
    }
    switch (detected)
    {
    case OPT_V:
        showBaseInfo(VERSION);
        goto coolpi_end;
    case OPT_H:
        showBaseInfo(NULL);
        goto coolpi_end;
    case OPT_C:
        openMiniLog(LOG_FILE); //This is quite important!
        reloadConf(0);
        goto core;
    case 0:
        openMiniLog(LOG_FILE); //This is quite important!
        goto core;
    default:
        showBaseInfo(NULL);
        goto coolpi_end;
    }
core:
    writeMiniLog(INFO, "Trying to run as linux daemon.");
    if (daemon(0, 1) == -1)
    {
        writeMiniLog(ERROR, "Error detected within daemon(), since multi-instances running at the same time is not allowed.");
        exit(1);
    }
    /*Check Pid first.*/
    checkPidBeforeRun();
    /*redirect the stdout and stdin*/
    freopen(NULL_DEVICE, "w", stdout);
    freopen(NULL_DEVICE, "r", stdin);
    /*Assign clean function.*/
    struct sigaction cleanAct;
    cleanAct.sa_handler = safeExit;
    cleanAct.sa_flags = 0;
    sigemptyset(&cleanAct.sa_mask);
    sigaction(SIGINT, &cleanAct, NULL);
    sigaction(SIGQUIT, &cleanAct, NULL);
    sigaction(SIGTERM, &cleanAct, NULL);
    /*Assign hot reload function.*/
    struct sigaction reload;
    reload.sa_handler = reloadConf;
    reload.sa_flags = 0;
    sigemptyset(&reload.sa_mask);
    sigaction(SIGHUP, &reload, NULL);
    /*Log currnet configure.*/
    show();
    /*entre the core function.*/
    run();
coolpi_end:
    return 0;
}
/*Offer feedback fuctions.*/
void showBaseInfo(const char *m_type)
{
    if (m_type != NULL)
    {
        printf("Usage:\n\tcoolpi [option] [value]\n");
        printf("[option]:\n\t[-v] [--verion]:\tshow version info.\n\t[-h] [--help]:\tshow details of options.\n\t[-c <file>] [--configure=<file>]:\tuse customed configure.\n");
        printf("I have offered a sample of customed configure file:\'/etc/coolpi/sample.json\'\n");
    }
    else
    {
        printf("coolpi %s\n", VERSION);
    }
}
