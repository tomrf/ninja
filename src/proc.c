#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "common.h"

int proc_is_root(const pinfo_t *pi)
{
    if (pi->uid == 0)   return 1;
    if (pi->euid == 0)  return 1;
    if (pi->suid == 0)  return 1;
    if (pi->fsuid == 0) return 1;
    if (pi->gid == 0)   return 1;
    if (pi->egid == 0)  return 1;
    if (pi->sgid == 0)  return 1;
    if (pi->fsgid == 0) return 1;
    return 0;
}


int proc_get_info(const pid_t pid, pinfo_t *pi)
{
    FILE         *fp;
    char         filename[32];
    char         rbuf[128];
    char         pbuf[5][17];
    int          found = 0;
    int          i;

    if (kill(pid, 0) != 0) {
        return -1;
    }

    snprintf(filename, sizeof filename, "/proc/%d/status", pid);

    if ((fp = fopen(filename, "r")) == NULL)
        return -2;

    while (fgets(rbuf, sizeof rbuf, fp) != NULL) {

        for (i = 0; i < 5; i++)
            pbuf[i][0] = '\0';

        sscanf(rbuf, "%16s %16s %16s %16s %16s", pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4]);

        if (!strncmp(pbuf[0], "Na", 2)) {
            snprintf(pi->name, 15, "%s", pbuf[1]);
            found++;
        }
        else if (!strncmp(pbuf[0], "Pi", 2)) {
            pi->pid = atoi(pbuf[1]);
            found++;
        }
        else if (!strncmp(pbuf[0], "PP", 2)) {
            pi->ppid = atoi(pbuf[1]);
            found++;
        }
        else if (!strncmp(pbuf[0], "Ui", 2)) {
            pi->uid = atoi(pbuf[1]);
            pi->euid = atoi(pbuf[2]);
            pi->suid = atoi(pbuf[3]);
            pi->fsuid = atoi(pbuf[4]);
            found++;
        }
        else if (!strncmp(pbuf[0], "Gi", 2)) {
            pi->gid = atoi(pbuf[1]);
            pi->egid = atoi(pbuf[2]);
            pi->sgid = atoi(pbuf[3]);
            pi->fsgid = atoi(pbuf[4]);
            found++;
        }

        if (found == 5)
            break;
    }

    fclose(fp);
    return 0;
}
