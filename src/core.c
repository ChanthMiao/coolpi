#include "core.h"
#include "miniLog.h"

CONF sharedConf = {
    .con = D_CON,
    .upperLimit = D_UPPER_LIMIT,
    .onMsec = D_ON_M_SEC,
    .lowerLimit = D_LOWER_LIMIT,
    .offMsec = D_OFF_M_SEC,
    .waitMsec = D_WAIT_M_SEC,
};
int currentTemperature = 0;
const char *conf_file = NULL;
/*Check pid in order to avoid multi-instances running.*/
int checkPidBeforeRun(void)
{
    int pidFile = -1;
    char buf[32];
    pidFile = open(PID_FILE, O_WRONLY | O_CREAT, 0666);
    if (pidFile < 0)
    {
        writeMiniLog(ERROR, "failed to open pid file\n");
        exit(EXIT_FAILURE);
    }
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    if (fcntl(pidFile, F_GETLK, &lock) < 0)
    {
        writeMiniLog(ERROR, "failed to fcntl F_GETLK\n");
        exit(EXIT_FAILURE);
    }
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(pidFile, F_SETLK, &lock) < 0)
    {
        writeMiniLog(ERROR, "failed to fcntl F_SETLK\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid = getpid();
    int len = snprintf(buf, 32, "%d\n", (int)pid);
    write(pidFile, buf, (size_t)len);
    return pid;
}

void safeExit(int sig)
{
    writeMiniLog(INFO, "Cleaning file descriptions.");
    int n = sysconf(_SC_OPEN_MAX);
    for (int i = 3; i < n; i++)
    {
        close(i);
    }
    exit(EXIT_SUCCESS);
}

/*Load default configure before the run() is callled.*/
void loadDefaultConf(void)
{
    writeMiniLog(INFO, "Loading default configure values.");
    sharedConf.con = D_CON;
    sharedConf.upperLimit = D_UPPER_LIMIT;
    sharedConf.onMsec = D_ON_M_SEC;
    sharedConf.lowerLimit = D_LOWER_LIMIT;
    sharedConf.offMsec = D_OFF_M_SEC;
    sharedConf.waitMsec = D_WAIT_M_SEC;
}

/*Try to read file content to a string. If failed, return null pointer.*/
static char *readFileToString(const char *filePath)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(filePath, &statbuff) < 0)
    {
        writeMiniLog(WARN, "failed to get the filesize of %s.", filePath);
        return NULL;
    }
    else
    {
        filesize = statbuff.st_size;
    }
    char *content = (char *)malloc(filesize);
    int file = open(filePath, O_RDONLY);
    if (file < 0)
    {
        writeMiniLog(WARN, "failed to open file (%s).", filePath);
        return NULL;
    }
    else
    {
        int n = read(file, content, filesize);
        close(file);
        if (n < 0)
        {
            writeMiniLog(WARN, "failed to get the content of %s.", filePath);
            return NULL;
        }
        else
        {
            return content;
        }
    }
}

/*Reload the configure from user's file. If the conf_file is unavailable, do nothing.*/
void reloadConf(int sig)
{
    char *confString = readFileToString(conf_file);
    if (confString == NULL)
    {
        writeMiniLog(WARN, "failed to get content of configure file. Use the default value instead.");
        return;
    }
    else
    {
        cJSON *root = cJSON_Parse(confString);
        free(confString); //Never delete this row.
        if (root == NULL)
        {
            writeMiniLog(ERROR, "error detected in cJSON_Parse(). Check the configure file please!");
            return;
        }
        else
        {
            const cJSON *con = NULL;
            const cJSON *upperLimit = NULL;
            const cJSON *onMsec = NULL;
            const cJSON *lowerLimit = NULL;
            const cJSON *offMsec = NULL;
            const cJSON *waitMsec = NULL;
            const cJSON *logFile = NULL;
            con = cJSON_GetObjectItemCaseSensitive(root, "con");
            if (cJSON_IsNumber(con) && con->valueint >= 0 && con->valueint <= 6)
            {
                sharedConf.con = con->valueint;
                writeMiniLog(INFO, "Updated the value of \"con \".");
            }
            upperLimit = cJSON_GetObjectItemCaseSensitive(root, "upperLimit");
            lowerLimit = cJSON_GetObjectItemCaseSensitive(root, "lowerLimit");
            if (cJSON_IsNumber(upperLimit) && cJSON_IsNumber(lowerLimit) && lowerLimit->valueint >= 0 && upperLimit->valueint > lowerLimit->valueint)
            {
                sharedConf.upperLimit = upperLimit->valueint;
                writeMiniLog(INFO, "Updated the value of \"upperLimit \".");
                sharedConf.lowerLimit = lowerLimit->valueint;
                writeMiniLog(INFO, "Updated the value of \"lowerLimit \".");
            }
            onMsec = cJSON_GetObjectItemCaseSensitive(root, "onMsec");
            if (cJSON_IsNumber(onMsec) && onMsec->valueint > 0)
            {
                sharedConf.onMsec = (unsigned int)onMsec->valueint;
                writeMiniLog(INFO, "Updated the value of \"onMsec \".");
            }
            offMsec = cJSON_GetObjectItemCaseSensitive(root, "offMsec");
            if (cJSON_IsNumber(offMsec) && offMsec->valueint > 0)
            {
                sharedConf.offMsec = (unsigned int)offMsec->valueint;
                writeMiniLog(INFO, "Updated the value of \"offMsec \".");
            }
            waitMsec = cJSON_GetObjectItemCaseSensitive(root, "waitMsec");
            if (cJSON_IsNumber(waitMsec) && waitMsec->valueint > 0)
            {
                sharedConf.waitMsec = (unsigned int)waitMsec->valueint;
                writeMiniLog(INFO, "Updated the value of \"waitMsec \".");
            }
            show();
            cJSON_Delete(root);
        }
    }
}

/* Core of the coolpi */
void run(void)
{
    /*Init wiringPi Pin control*/
    wiringPiSetup();
    pinMode(sharedConf.con, OUTPUT);

    int temperatureFile = -1;
    temperatureFile = open(TEMPERATURE_FILE, O_RDONLY);
    if (temperatureFile < 0)
    {
        writeMiniLog(ERROR, "failed to open temperature file.");
        exit(EXIT_FAILURE);
    }

    int isOpen;
    char tmp[8];
    memset(tmp, 0, 8);
    while (1)
    {
        isOpen = !digitalRead(sharedConf.con);
        lseek(temperatureFile, 0, SEEK_SET);
        if (read(temperatureFile, tmp, 8) == -1)
        {
            writeMiniLog(ERROR, "failed to get the current CPU temperature.");
        }
        else
        {
            currentTemperature = atoi(tmp);
        }
        if (currentTemperature >= sharedConf.upperLimit)
        {
            digitalWrite(sharedConf.con, LOW);
            if (!isOpen)
            {
                writeMiniLog(INFO, "%d\tOn", currentTemperature / 1000);
            }
            delay(sharedConf.onMsec);
        }
        else if (currentTemperature <= sharedConf.lowerLimit)
        {
            digitalWrite(sharedConf.con, HIGH);
            if (isOpen)
            {
                writeMiniLog(INFO, "%d\tOff", currentTemperature / 1000);
            }
            delay(sharedConf.offMsec);
        }
        else
        {
            delay(sharedConf.waitMsec);
        }
    }
}

/*Show current configure in log.*/
void show(void)
{
    writeMiniLog(INFO, "con=%u, upperLimit=%d, onMsec=%u, lowerLimit=%d, offMsec=%u, waitMsec=%u.", sharedConf.con, sharedConf.upperLimit, sharedConf.onMsec, sharedConf.lowerLimit, sharedConf.offMsec, sharedConf.waitMsec);
}
