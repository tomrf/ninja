#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include "common.h"
#include "proc.h"

extern struct options global_opts;

void main_loop(int mgroup, int cycledelay, char *wlist)
{
  int root_pids[PID_MAX];
  int *active_pids;
  int i, j;
  int rinitw_trap;
  int file_mess;
  struct proc_info pi;
  struct proc_info ppi;
  pid_t evil_pid;
  pid_t evil_ppid;
  char evil_pid_name[FILENAME_MAX+1];
  char evil_ppid_name[FILENAME_MAX+1];

  LOG("generating initial pid array..");

  /* run through all possible pids and set up our inital pid array */

  for (i = 0; i < PID_MAX; i++) {
    root_pids[i] = 0;
    if ( (get_proc_info(i, &pi) != -1) && (pi.uid == 0 || pi.gid == 0) )
      root_pids[i] = 1;
  }

  LOG("now monitoring process activity");

  /* this is the main loop and we should never abort from it.. */
  for (;;) {

    active_pids = get_active_pids();

    /* run though all possible pids.. */

    for (i = global_opts.soffset; i < PID_MAX; i++) {

      if (active_pids[i] == 0)
        continue;

      /* look for pids running as root who are not already in our root
       * pid array */

      if ( (get_proc_info(i, &pi) != -1) && (pi.uid == 0 || pi.gid == 0) ) {

        if (root_pids[i] != 1) {

          /* we found a new unknown root process, let's gather and log some
           * information about it and its parent */

          root_pids[i] = 1;

          /* XXX: this should perhaps be handled better */
          if (get_proc_info(pi.ppid, &ppi) == -1) {
            root_pids[i] = 0;
            continue;
          }

          /* require processes controlled by init to be whitelisted? */
          rinitw_trap = 0;
          if ( (global_opts.rinitw == TRUE) && (pi.ppid == 1) && (!wlist_match(wlist, pi.pid, ppi.uid, ppi.gid)) ) {

            LOG("INIT CONTROLLED PROCESS NOT WHITELISTED -- JUMPING TO KILL");
            rinitw_trap = 1;

          } else {

            /* ignore root spawned process? */
            if (ppi.uid == 0 && global_opts.noroot == TRUE)
              goto loop_end;

            /* ignore whitelisted process? */
            if (global_opts.wlist[0] != '\0' && wlist_match(wlist, pi.pid, ppi.uid, ppi.gid)) {

              if (global_opts.logwlist == TRUE)
                LOG("WHITELIST: %s[%d] ppid_uid=%d", pi.name, pi.pid, ppi.uid);

              goto loop_end;
            }

          }

          LOG("NEW ROOT PROCESS: %s[%d] ppid=%d uid=%d gid=%d",
              pi.name, pi.pid, pi.ppid, pi.uid, pi.gid);

          evil_pid = pi.pid;
          evil_ppid = pi.ppid;

          snprintf(evil_pid_name, FILENAME_MAX, "%s", pi.name);

          if (get_proc_info(pi.ppid, &pi) != -1) {
  
            LOG("  - ppid uid=%d(%s) gid=%d ppid=%d", pi.uid, 
                 get_user(pi.uid), pi.gid, pi.ppid);

            snprintf(evil_ppid_name, FILENAME_MAX, "%s", pi.name);

            if (get_user(pi.uid) == NULL)
              LOG("  ! WARNING: unable to get username for uid %d", pi.uid);

            /* sanity check passwd and group files */
            file_mess = 0;
            if (file_isregular("/etc/passwd") != 1) {
              LOG("  ! WARNING: /etc/passwd has been modified: no longer a regular file");
              file_mess++;
            }
            if (file_isregular("/etc/group") != 1) {
              LOG("  ! WARNING: /etc/group has been modified: no longer a regular file");
              file_mess++;
            }
            if (file_size("/etc/passwd") < 1) {
              LOG("  ! WARNING: /etc/passwd has a size of 0 bytes");
              file_mess++;
            }

            if (file_mess != 0) {
              LOG("  + Detected problems with passwd and/or group file, jumping to kill");
              goto kill;
            }

            /* check if the user who spawned this root process is in our
             * "magic group" */

            if (!check_group(get_user(pi.uid), mgroup) && rinitw_trap == 0) {
              LOG("  + %s is in magic group, all OK!", get_user(pi.uid));
            }
            else {

              /* the user was NOT in our magic group.. let's kill the
               * offending process, and its parent */
kill:
              LOG("  + UNAUTHORIZED PROCESS DETECTED: %s[%d] (parent: %s[%d])",
                  evil_pid_name, evil_pid, evil_ppid_name, evil_ppid);

              if (global_opts.nokill == FALSE) {

                LOG("  - sending signal SIGKILL to pid %d", evil_pid);
                kill(evil_pid, SIGKILL);

                if (global_opts.nokillppid == FALSE && rinitw_trap == 0) {
                
                  LOG("  - sending signal SIGKILL to ppid %d", evil_ppid);
                  kill(evil_ppid, SIGKILL);

                }

                LOG("  * offending process(es) terminated");

              } else {

                LOG("  - nokill option set, no signals sent");

              }

              if (global_opts.command[0] != 0x00) {

                char extcmd[256];

                snprintf(extcmd, sizeof extcmd, "%s %s", global_opts.command,
                    get_user(pi.uid));

                LOG("  - executing '%s'", extcmd);

                switch(fork()) {
                
                  case 0:
                    system(extcmd);
                    _exit(EXIT_SUCCESS);
                    break;

                  case -1:
                    LOG("  - ERROR: fork() failed when trying to execute command");
                    break;

                }

              }

              /* .. */
              
            }

          }

        }

      } 

    }

    loop_end:

    /* remove lost root pids form root pid array */

    for (j = 0; j < PID_MAX; j++) {
      if (root_pids[j] == 1) {
        if (active_pids[j] != 1)
          root_pids[j] = 0;
      }
    }

    /* sleep */

    if (cycledelay <= 0)
      usleep(50000);
    else 
      sleep(cycledelay);

  }

}


int* get_active_pids(void)
{
  DIR *dp;
  struct dirent *d;
  static int active_pids[PID_MAX];
  int pidnr;
  int i;

  if ((dp = opendir("/proc")) == NULL)
    return NULL;

  for (i = 0; i < PID_MAX; i++)
    active_pids[i] = 0;

  while ((d = readdir(dp)) != NULL) {

    pidnr = strtol(d->d_name, NULL, 0);

    if (pidnr > 0)
      active_pids[pidnr] = 1;

  }

  closedir(dp);

  return active_pids;
}
