#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "common.h"


int kill_children(const pid_t pid)
{
    pinfo_t   pi;
    int       max_pid;
    int       count = 0;
    int       ret;
    int       i;

    if (pid == 1) {
        printf("EEEEEEEEEK, will not kill children of pid 1\n");
        return -1;
    }

    max_pid = get_max_pid();

    for (i = 0; i < max_pid; i++) {
        ret = proc_get_info(i, &pi);
        if (ret == 0 && pi.ppid == pid) {
            printf(" - check it: %d is a child of %d !\n", pi.pid, pid);
            printf("kill(%d, 9)\n", pi.pid);
            kill(pi.pid, 9);
        }
    }

    return count;
}


int check_proc(const pinfo_t *pi, const pinfo_t *ppi)
{
    uinfo_t     uinfo;
    int         ret;
    char        link_path[32];
    char        path[PATH_MAX + 8];

    if (kill(pi->pid, 0) != 0)
        return -1;


    LOG(VERBOSE, "process:%s[%d] uids:%d,%d,%d,%d gids:%d,%d,%d,%d",
            pi->name, pi->pid, pi->uid, pi->euid, pi->suid, pi->fsuid,
            pi->gid, pi->egid, pi->sgid, pi->fsgid);
    LOG(VERBOSE, "parent:%s[%d] uids:%d,%d,%d,%d gids:%d,%d,%d,%d",
            ppi->name, ppi->pid, ppi->uid, ppi->euid, ppi->suid, ppi->fsuid,
            ppi->gid, ppi->egid, ppi->sgid, ppi->fsgid);

    /* get executable path */
    memset(path, '\0', sizeof path);
    snprintf(link_path, sizeof link_path, "/proc/%d/exe", pi->pid);
    readlink(link_path, path, sizeof path - 1); /* xxx: handle error */

    user_get_info(ppi->uid, &uinfo);
    /* xxx: handle error */
    
    /* check: ignore root spawned procs? */
    if (ppi->uid == 0 && ppi->pid != 1) {
        printf("ignoring root spawned proc, pid: %d\n", pi->pid);
        ret = 0;
    }
    /* check: whitelist matching */
    else if (wlist_match(&uinfo, path)) {
        printf("wlist match\n");
        ret = 0;
    }
    /* alert! */
    else {
        printf("ALERT!\n");
        printf("kill_children(%d)..\n", pi->pid);
        kill_children(pi->pid);
        printf("kill(%d, 9)\n", pi->pid);
        kill(pi->pid, 9);
        ret = 1;
    }

    if (uinfo.groups != NULL)
        free(uinfo.groups);

    return ret;
}
