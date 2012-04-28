#ifndef _COMMON_H
#define _COMMON_H

#include <linux/limits.h>

#define PROG_NAME     "ninja"
#define PROG_VERSION  "ninja-1.0.0-devel"

#define TRUE  1
#define FALSE 0

#define ALERT   0
#define ERROR   0
#define WARNING 1
#define INFO    2
#define VERBOSE 3
#define DEBUG   4

#define MAX_USERLEN 32

typedef struct proc_info {
    pid_t    pid;
    pid_t    ppid;
    uid_t    uid, euid, suid, fsuid;
    gid_t    gid, egid, sgid, fsgid;
    char     name[16];
} pinfo_t;

typedef struct user_info {
    uid_t    uid;
    gid_t    gid;
    char     name[MAX_USERLEN + 1];
    gid_t    *groups;
    size_t   ngroups;
} uinfo_t;

typedef struct wlist_node {
    char     path[PATH_MAX];
    uid_t    *uids;
    gid_t    *gids;
    size_t   nuids, ngids;
    struct wlist_node *next;
} wlnode_t;

#include "proto.h"

#endif
