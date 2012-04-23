#include "proto.h"
#include "config.h"

#define PROG_NAME     "ninja"
#define PROG_VERSION  "0.1.3"

#define TRUE  1
#define FALSE 0

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
  char  logfile[128];
  char  command[128];
  char  wlist[128];
  int   soffset;
};

#ifndef PID_MAX
  #define PID_MAX 0x8000   /* found in linux/threads.h */
#endif

#ifndef FILENAME_MAX
  #define FILENAME_MAX 255
#endif
