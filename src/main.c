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
#include "grp.h"


#define DEFAULT_CYCLE_DELAY  1

struct options global_opts;


int main(int argc, char **argv)
{
  int i;
  struct group *gr;
  FILE *fd;
  char *wlist = NULL;
 
  global_opts.daemon = FALSE;
  global_opts.interval = DEFAULT_CYCLE_DELAY;
  global_opts.quiet = FALSE;
  global_opts.group = 0;
  global_opts.nokill = FALSE;
  global_opts.nokillppid = FALSE;
  global_opts.noroot = TRUE;
  global_opts.logwlist = FALSE;
  global_opts.rinitw = FALSE;
  global_opts.soffset = 0;
  memset(global_opts.logfile, 0x00, sizeof global_opts.logfile);
  memset(global_opts.command, 0x00, sizeof global_opts.command);
  memset(global_opts.wlist, 0x00, sizeof global_opts.wlist);


  if (argc <= 1)
    LOG("warning: no configuration file specified, using default values");

  if (argc > 1 && argv[1][0] != '-') {
    LOG("reading configuration file: %s", argv[1]);

    if (conf_read(argv[1]) != 0)
      die("error: unable to read configuration file");

  } else {

    if (argc > 1 && !strcmp(argv[1], "-h")) {
      help();
      exit(EXIT_FAILURE);
    }

  }

  LOG("ninja version %s initializing", PROG_VERSION);

  if (getuid() != 0)
    die("error: must run as root");

  /* sanity check /etc/passwd and /etc/group */
  if (file_isregular("/etc/passwd") != 1)
    LOG("warning: /etc/passwd is not a regular file -- this will cause problems");
  if (file_isregular("/etc/group") != 1)
    LOG("warning: /etc/group is not a regular file -- this will cause problems");
  if (file_size("/etc/passwd") < 1)
    LOG("warning: /etc/passwd has a size of 0 bytes -- this will cause problems");

  if (global_opts.group == -1)
    die("error: no magic group specified");

  if (global_opts.rinitw == TRUE && global_opts.wlist[0] == 0x00)
    die("error: require_init_wlist option set, but no whitelist is specified");

  for (i = 0; i < 100; i++)
    signal(i, sighandler);

  if ((gr = getgrgid(global_opts.group)) == NULL)
    die("error: magic group not found by getgrgid()");

  LOG("magic group: gid=%d (%s)", gr->gr_gid, gr->gr_name);

  if (global_opts.logfile[0] != 0x00) {

    LOG("logfile: %s", global_opts.logfile);

    if (file_isregular(global_opts.logfile) != 1)
      die("error: `%s' is not a regular file", global_opts.logfile);

  /*  Fortify shows possible race condition here.
  **  Avoid locating the log file in a directory where a user
  **  might delete it and replace it with a link to a system file
  **  that they want to overwrite.
  */
    if ((fd = fopen(global_opts.logfile, "a")) == NULL)
      die("error: unable to open log file");
    else
      fclose(fd);
  }

  if (global_opts.command[0] != 0x00) {
    LOG("external command: %s <user>", global_opts.command);
  }

  if (global_opts.wlist[0] != 0x00) {
    if ((wlist = wlist_read(global_opts.wlist)) == NULL)
      die("failed to read whitelist");
    LOG("whitelist mapped in memory at %p", (void *)wlist);
  }

  if (global_opts.soffset != 0) {
    LOG("proc scanning offset: %d", global_opts.soffset);
  }

  if (global_opts.daemon == TRUE) {
    LOG("entering daemon mode");
    if (daemon(0, 0) == -1)
      die("error: daemon() failed");
    global_opts.quiet = TRUE;
  }

  LOG("entering main loop");

  main_loop(global_opts.group, global_opts.interval, wlist);

  return EXIT_SUCCESS;
}


void help(void)
{
  printf("usage: %s <config_file>\n", PROG_NAME);
}
