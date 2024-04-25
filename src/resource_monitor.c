#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
 
struct CPUStats 
{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
};

struct IOStats 
{
    unsigned long long read;
    unsigned long long write;
};

struct NetworkStats 
{
    unsigned long long rec_bytes;
    unsigned long long trans_bytes;
};

void logResourceUsage(FILE *logFile, struct CPUStats *prevCpuStats, struct IOStats *prevIOStats, struct NetworkStats *prevNetStats, int interval) 
{

    FILE *cpuFile = fopen("/proc/stat", "r");
    FILE *memFile = popen("free | awk '/Mem/{printf \"%.2f\", ($2-$7)/$2 * 100}'", "r");
    FILE *ioFile = fopen("/proc/diskstats", "r");
    FILE *netFile = fopen("/proc/net/dev", "r");

    // eroare la deschidere de fisier
    if (cpuFile == NULL || memFile == NULL || ioFile == NULL || netFile == NULL) 
    {
        perror("eroare la deschidere de fisier");
        exit(EXIT_FAILURE);
    }
    
    //CPU Usage
    char cpuLine[256];
    while (fgets(cpuLine, sizeof(cpuLine), cpuFile) != NULL) 
    {
        if (strncmp(cpuLine, "cpu ", 4) == 0) 
        {
            break;
        }
    }
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    sscanf(cpuLine, "cpu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long totalDiff = total - (prevCpuStats->user + prevCpuStats->nice + prevCpuStats->system + prevCpuStats->idle + prevCpuStats->iowait + prevCpuStats->irq + prevCpuStats->softirq);
    unsigned long long idleDiff = idle - prevCpuStats->idle;
    prevCpuStats->user = user;
    prevCpuStats->nice = nice;
    prevCpuStats->system = system;
    prevCpuStats->idle = idle;
    prevCpuStats->iowait = iowait;
    prevCpuStats->irq = irq;
    prevCpuStats->softirq = softirq;
    double cpuUsage = (totalDiff - idleDiff) * 100.0 / totalDiff;
    if (cpuUsage > 100.0) 
    {
        cpuUsage = 100.0;
    }
    fclose(cpuFile);

    // I/O: KB scrisi si cititi per sec
    char ioLine[256];
    while (fgets(ioLine, sizeof(ioLine), ioFile) != NULL) 
    {
        if (strstr(ioLine, "loop0")) 
        {
            break;
        }
    }
    unsigned long long read, write;
    sscanf(ioLine, "   %*u       %*u loop0 %*u %*u %llu %*u %*u %*u %llu", &read, &write); //sectoare scrise si citite
    unsigned long long readDiff = read - prevIOStats->read;
    unsigned long long writeDiff = write - prevIOStats->write;
    prevIOStats->read = read;
    prevIOStats->write = write;
    double ioUsage = ((readDiff + writeDiff) * 512)/1024; //un sector are 512Bytes
    //float ioUsage = (readDiff + writeDiff) * 100.0 / (interval * 1024);
    fclose(ioFile);

    // bandwidth in bytes
    char netLine[256];
    while (fgets(netLine, sizeof(netLine), netFile) != NULL) 
    {
        if (strstr(netLine, "wlan0")) 
        {
            break;
        }
    }
    unsigned long long rec_bytes;
    unsigned long long trans_bytes;
    //sscanf(netLine, "enp0s3: %llu", &rec_bytes);
    sscanf(netLine, " wlan0: %llu %*u %*u %*u %*u %*u %*u %*u %llu", &rec_bytes, &trans_bytes);
    unsigned long long recBytesDiff = rec_bytes - prevNetStats->rec_bytes;
    unsigned long long transBytesDiff = trans_bytes - prevNetStats->trans_bytes;
    prevNetStats->rec_bytes = rec_bytes;
    prevNetStats->trans_bytes = trans_bytes;
    fclose(netFile);

    // Usage memorie
    float memUsage;
    fscanf(memFile, "%f", &memUsage);
    if (memUsage > 100.0) {
        memUsage = 100.0;
    }
    pclose(memFile);

    //timp curent si printare
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    fprintf(logFile, "%04d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.1f,%llu,%llu\n",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
            cpuUsage, memUsage, ioUsage, recBytesDiff, transBytesDiff);
}

int main() 
{
    FILE *logFile;
    logFile = fopen("resource_log.csv", "a");

    // eroare la decshidere de fisier
    if (logFile == NULL) {
        perror("eroare la deschidere fisier de log");
        exit(EXIT_FAILURE);
    }

    // pentru a preveni ca datele prev sa nu fie initializate si sa fie eronate pe prima linie in csv
    struct CPUStats prevCpuStats = {0};
    struct NetworkStats prevNetStats = {0};
    FILE *netFile = fopen("/proc/net/dev", "r");
    if (netFile == NULL) 
    {
        perror("eroare la deschiderea /proc/net/dev");
        exit(EXIT_FAILURE);
    }
    char netLine[256];
    while (fgets(netLine, sizeof(netLine), netFile) != NULL) 
    {
        if (strstr(netLine, "wlan0")) 
        {
            //sscanf(netLine, "enp0s3: %llu", &prevNetStats.rec_bytes);
            sscanf(netLine, " wlan0: %llu %*u %*u %*u %*u %*u %*u %*u %llu", &prevNetStats.rec_bytes, &prevNetStats.trans_bytes);
            break;
        }
    }
    fclose(netFile);
    struct IOStats prevIOStats;
    FILE *ioFile = fopen("/proc/diskstats", "r");
    if (ioFile == NULL) 
    {
        perror("eroare la deschiderea /proc/diskstats");
        exit(EXIT_FAILURE);
    }
    char ioLine[256];
    while (fgets(ioLine, sizeof(ioLine), ioFile) != NULL) 
    {
        if (strstr(ioLine, "loop0")) 
        {
            sscanf(ioLine, "   %*u       %*u loop0 %*u %*u %llu %*u %*u %*u %llu", &prevIOStats.read, &prevIOStats.write);
            break;
        }
    }
    fclose(ioFile);
    
    // interval de rulare + rulare
    int interval = 180;
    time_t end_time = time(NULL) + interval;
    while (time(NULL) < end_time) 
    { 
        logResourceUsage(logFile, &prevCpuStats, &prevIOStats, &prevNetStats, interval);
        sleep(1); 
    }
    fclose(logFile);

    return 0;
}
