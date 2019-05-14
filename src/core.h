#ifndef _CORE_H_
#define _CORE_H_

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU 1
#endif

/*offer default values*/
#define D_CON 4
#define D_UPPER_LIMIT 48000
#define D_ON_M_SEC 20000
#define D_LOWER_LIMIT 44000
#define D_OFF_M_SEC 2000
#define D_WAIT_M_SEC 1000
#define PID_FILE "/var/run/coolpi.pid"
#define NULL_DEVICE "/dev/null"
#define TEMPERATURE_FILE "/sys/class/thermal/thermal_zone0/temp"

/*Define the struct of configure*/
typedef struct configure
{
    int con;
    int upperLimit;
    unsigned onMsec;
    int lowerLimit;
    unsigned offMsec;
    unsigned waitMsec;
} CONF;
/*Offer the prototype*/

/*Check pid in order to avoid multi-instances running*/
int checkPidBeforeRun(void);
void safeExit(int sig);
/*Load default configure before the run() is callled*/
void loadDefaultConf(void);
/*Reload the configure from user's file. If the conf_file is unavailable, do nothing*/
void reloadConf(int sig);
/*Core of the coolpi*/
void run(void);
/*Show current configure in log.*/
void show(void);

#endif
