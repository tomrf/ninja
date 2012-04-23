#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include "common.h"


#define DIE_PREFIX    "die: "
#define LOG_PREFIX    "log: "

extern struct options global_opts;

void LOG(char *fmt, ...)
{
  va_list args;
  FILE *fd;

  if (global_opts.quiet == FALSE) {
    va_start(args, fmt);
#ifdef LOG_PREFIX
    fprintf(stdout, "%s", LOG_PREFIX);
#endif
    vfprintf(stdout, fmt, args);
    puts("");
    va_end(args);
  }

  /*  Fortify shows possible race condition here.
  **  Avoid locating the log file in a directory where a user
  **  might delete it and replace it with a link to a system file
  **  that they want to overwrite.
  */
  if (global_opts.logfile[0] != 0x00 && file_isregular(global_opts.logfile) == 1) {
    va_start(args, fmt);
    fd = fopen(global_opts.logfile, "a");
    if (fd != NULL) {
      fprintf(fd, "[%s ", replace(timestamp(), '\n', ']'));
      vfprintf(fd, fmt, args);
      fprintf(fd, "\n");
      fclose(fd);
    }
    va_end(args);
  }
  return;
}


void die(char *fmt, ...)
{
  va_list args;

#ifdef DIE_PREFIX
  fprintf(stderr, "%s", DIE_PREFIX);
#endif

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);

  va_end(args);

  puts("");

  exit(EXIT_FAILURE);
}


char* timestamp(void)
{
  time_t t = time(0);

  return ctime(&t);
}


char* replace(char *string, char c1, char c2)
{
  int i;

  for (i = 0; i < strlen(string); i++)
    if (string[i] == c1)
      string[i] = c2;

  return string;
}
