#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"


pid_t get_max_pid(void)
{
    static pid_t   max_pid = -1;
    FILE           *fp;

    if (max_pid == -1) {
        fp = fopen("/proc/sys/kernel/pid_max", "r");
        if (fp) {
            fscanf(fp, "%d", &max_pid);
            fclose(fp);
        }
        if (max_pid < 0x8000)
            max_pid = 0x8000;
    }

    return max_pid;
}


int main(void)
{
    pid_t         max_pid;
    pinfo_t       pi, parent;
    int           *root_pids;
    int           ret;
    int           i;

    
    wlist_load("./wlist.txt");
    wlist_debug_dump_list();



    /* get max_pid */
    max_pid = get_max_pid();
    LOG(DEBUG, "max_pid=%d", max_pid);

    /* allocate arrays */
    root_pids = calloc(max_pid, sizeof (int));        /* xxx: handle error */

    /* build initial root pid list */
    for (i = 0; i < max_pid; i++) {
        *(root_pids + i) = 0;
        ret = proc_get_info(i, &pi);
        if (ret == 0 && proc_is_root(&pi))
            *(root_pids + i) = 1;
    }

    /* monitor loop */
    while (1) {
        for (i = 0; i < max_pid; i++) {
            ret = proc_get_info(i, &pi);
            if (ret != 0) {
                *(root_pids + i) = 0;
            }
            else if (proc_is_root(&pi) && *(root_pids + i) != 1) {
                proc_get_info(pi.ppid, &parent);
                check_proc(&pi, &parent);
                *(root_pids + i) = 1;
            }
        }
        usleep(500 * 1000);
    }

    return EXIT_SUCCESS;
}
