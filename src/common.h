#include "proto.h"
#include "config.h"

#define PROG_NAME     "ninja"
#define PROG_VERSION  "0.1.4"

#define TRUE  1
#define FALSE 0

#define MAX_PATH 128

struct options {
  int   daemon;
  int   interval;
  int   quiet;
  int   nokill;
  int   nokillppid;
  int   noroot;
  int   logwlist;
  int   rinitw;
  gid_t group;
  char  logfile[MAX_PATH];
  char  command[MAX_PATH];
  char  wlist[MAX_PATH];
  int   soffset;
};

#ifndef PID_MAX
  #define PID_MAX 0x8000   /* found in linux/threads.h */
#endif

#ifndef FILENAME_MAX
  #define FILENAME_MAX 255
#endif
