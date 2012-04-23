#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "proc.h"


/* get_proc_info() -- parse /proc/<PID>/status for a given pid */

/* this should be pretty self explained.. we parse /proc/<PID>/status and
 * put the name, pid, ppid, uid and gid fields into a structure (proc_info) */

int get_proc_info(const pid_t pid, struct proc_info *pi)
{
  FILE *fp;
  char filename[32];
  char rbuf[128];
  char pbuf[5][33];
  int i;

  snprintf(filename, sizeof filename, "/proc/%d/status", pid);

  if ((fp = fopen(filename, "r")) == NULL)
    return -1;

  while (fgets(rbuf, sizeof rbuf, fp) != NULL) {

    for (i = 0; i < 5; i++)
      memset(pbuf[i], 0x00, sizeof pbuf[i]);

    sscanf(rbuf, "%32s %32s %32s %32s %32s", pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4]);

    if (!strcmp(pbuf[0], "Name:"))
      snprintf(pi->name, 127, "%s", pbuf[1]);

    if (!strcmp(pbuf[0], "Pid:"))
      pi->pid = atoi(pbuf[1]);

    if (!strcmp(pbuf[0], "PPid:"))
      pi->ppid = atoi(pbuf[1]);

    if (!strcmp(pbuf[0], "Uid:")) {

      pi->uid = atoi(pbuf[1]);

      for (i = 1; i < 4; i++) {
        if (atoi(pbuf[i]) == 0)
          pi->uid = 0;
      }

    }

    if (!strcmp(pbuf[0], "Gid:")) {

      pi->gid = atoi(pbuf[1]);

      for (i = 1; i < 4; i++) {
        if (atoi(pbuf[i]) == 0)
          pi->gid = 0;
      }

    }

  }

  fclose(fp);

  return 0;
}
