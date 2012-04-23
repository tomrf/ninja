#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

extern struct options global_opts;

int conf_read(const char *path)
{
  FILE *fp;
  char cf_option[65];
  char cf_eq;
  char cf_value[161];
  char buf[256];
  unsigned long line_num = 0;
  int i;
  int ignore_line;

  if ((fp = fopen(path, "r")) == NULL)
    return -1;

  for (;;) {

    memset(buf, 0x00, sizeof buf);
    if (fgets(buf, sizeof buf, fp) == NULL)
      break;

    line_num++;

    ignore_line = 0;
    for (i = 0; i < strlen(buf); i++) {

      if (buf[i] == ' ' || buf[i] == '\t')
        continue;

      if (buf[i] == '#' || buf[i] == '\n')
        ignore_line = 1;

      break;
    }

    if (ignore_line == 1)
      continue;

    cf_option[0] = 0x00;
    cf_eq = 0x00;
    cf_value[0] = 0x00;

    sscanf(buf, "%64s %c %160s\n", cf_option, &cf_eq, cf_value);

    stolower(cf_option, 65);
    stolower(cf_value, 161);

    if (!strcmp(cf_value, "true"))
      sprintf(cf_value, "yes");
    if (!strcmp(cf_value, "false"))
      sprintf(cf_value, "no");

    if (!strcmp(cf_option, "daemon") && !strcmp(cf_value, "yes"))
      global_opts.daemon = TRUE;
    else if (!strcmp(cf_option, "daemon") && !strcmp(cf_value, "no"))
      global_opts.daemon = FALSE;

    else if (!strcmp(cf_option, "interval"))
      global_opts.interval = atoi(cf_value);

    else if (!strcmp(cf_option, "group"))
      global_opts.group = atoi(cf_value);

    else if (!strcmp(cf_option, "logfile"))
      snprintf(global_opts.logfile, sizeof global_opts.logfile, "%s", cf_value);

    else if (!strcmp(cf_option, "whitelist"))
      snprintf(global_opts.wlist, sizeof global_opts.wlist, "%s", cf_value);

    else if (!strcmp(cf_option, "external_command"))
      snprintf(global_opts.command, sizeof global_opts.command, "%s", cf_value);

    else if (!strcmp(cf_option, "proc_scan_offset"))
      global_opts.soffset = atoi(cf_value);

    else if (!strcmp(cf_option, "no_kill") && !strcmp(cf_value, "yes"))
      global_opts.nokill = TRUE;
    else if (!strcmp(cf_option, "no_kill") && !strcmp(cf_value, "no"))
      global_opts.nokill = FALSE;

    else if (!strcmp(cf_option, "no_kill_ppid") && !strcmp(cf_value, "yes"))
      global_opts.nokillppid = TRUE;
    else if (!strcmp(cf_option, "no_kill_ppid") && !strcmp(cf_value, "no"))
      global_opts.nokillppid = FALSE;

    else if (!strcmp(cf_option, "ignore_root_procs") && !strcmp(cf_value, "yes"))
      global_opts.noroot = TRUE;
    else if (!strcmp(cf_option, "ignore_root_procs") && !strcmp(cf_value, "no"))
      global_opts.noroot = FALSE;

    else if (!strcmp(cf_option, "log_whitelist") && !strcmp(cf_value, "yes"))
      global_opts.logwlist = TRUE;
    else if (!strcmp(cf_option, "log_whitelist") && !strcmp(cf_value, "no"))
      global_opts.logwlist = FALSE;

    else if (!strcmp(cf_option, "require_init_wlist") && !strcmp(cf_value, "yes"))
      global_opts.rinitw = TRUE;
    else if (!strcmp(cf_option, "require_init_wlist") && !strcmp(cf_value, "no"))
      global_opts.rinitw = FALSE;
    
    else
      LOG("warning: %s:%d: unknown option and/or value", path, line_num);

  }

  return 0;
}


void stolower(char *s, int lim)
{
  int i;

  for (i = 0; i < strlen(s) && i < lim; i++)
    s[i] = tolower(s[i]);

}
