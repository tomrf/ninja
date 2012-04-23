#ifndef _PROC_H
#define _PROC_H

struct proc_info {
  pid_t pid;
  pid_t ppid;
  uid_t uid;
  gid_t gid;
  char  name[128];
};

#endif

