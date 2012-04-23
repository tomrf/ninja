#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <linux/limits.h>
#include "common.h"


off_t fd_len(int fd)
{
  off_t len;

  len = lseek(fd, 0, SEEK_END);
  (void) lseek(fd, 0, SEEK_SET);

  return len;
}  


char* wlist_read(char *path)
{
  int wlist_fd;
  char *wlist;

  if ((wlist_fd = open(path, O_RDONLY)) == -1)
    return NULL;

  if ((wlist = mmap(0, fd_len(wlist_fd), PROT_READ, MAP_PRIVATE, wlist_fd, 0)) == MAP_FAILED)
    return NULL;

  return wlist;
}


char* get_map_exec_offset(void)
{
  FILE *fp;
  static char buf[16] = "\0";

  if (!buf[0] == '\0')
    return buf;

  if ((fp = fopen("/proc/1/maps", "r")) == NULL) {
    LOG("warning: unable to get map_exec_offset from /proc/1/maps: whitelisting will not work");
    return NULL;
  }

  (void) fgets(buf, 9, fp);

  fclose(fp);

  return buf;
}


char* get_map_name(pid_t pid)
{
  FILE *fd;
  char mfile[64];
  char mfield[5][25];
  char mbuf[PATH_MAX+8+(25*5)+1];
  static char map_name[PATH_MAX+8+(25*5)+1];
 
  snprintf(mfile, sizeof mfile, "/proc/%d/maps", pid);

  if ((fd = fopen(mfile, "r")) == NULL)
    return NULL;

  for (;;) {

    memset(map_name, '\0', sizeof map_name);
    memset(mbuf, '\0', sizeof mbuf);
 
    if (fgets(mbuf, sizeof mbuf, fd) == NULL)
      return NULL;

    sscanf(mbuf, "%24s %24s %24s %24s %24s %s", mfield[0], mfield[1],
       mfield[2], mfield[3], mfield[4], map_name);

    if (!map_name[0]) {
      /* return NULL; */
      continue;
    }

    if (!strncmp(mfield[0], get_map_exec_offset(), 8))
      return map_name;

  }

  return NULL;
}


int wlist_match(char *wlist, pid_t pid, uid_t uid, gid_t gid)
{
  unsigned long wlist_pos = 0;
  int i = 0;

#define MAX_LINK_PATH	64
#ifdef USE_READLINK
  char map_name[PATH_MAX+8];
  char link_path[MAX_LINK_PATH];
#else
  char *map_name;
#endif

  char wlist_entry[512];
  char wlist_exec[512];
  char wlist_groups[512];
  char wlist_users[512];

#ifdef USE_READLINK
  memset(map_name, '\0', sizeof map_name);
  snprintf(link_path, sizeof link_path, "/proc/%d/exe", pid);
  if (readlink(link_path, map_name, sizeof map_name - 1) == -1)
    return 0;
#else
  if ((map_name = get_map_name(pid)) == NULL)
    return 0;
#endif

  for (;;) {

    wlist_entry[i++] = wlist[wlist_pos++];

    if (wlist[wlist_pos] == '\0')
      return 0;

    if (wlist[wlist_pos] == '\n') {
      wlist_entry[i] = '\0';

      memset(wlist_exec, '\0', sizeof wlist_exec);
      memset(wlist_groups, '\0', sizeof wlist_groups);
      memset(wlist_users, '\0', sizeof wlist_users);

      chreplace(wlist_entry, ':', ' ');
      sscanf(wlist_entry, "%511s %511s %511s", wlist_exec, wlist_groups, wlist_users);

      /* map name (executable) matces */
      if (strlen(wlist_exec) < PATH_MAX+8  &&
	      !strncmp(map_name, wlist_exec, PATH_MAX+8)) {

        /* check users */

        if (user_match(get_user(uid), wlist_users) > 0)
          return 1;

        /* check groups */

        if (group_match(uid, wlist_groups, gid) > 0)
          return 1;

      }

      i = 0;
      wlist_pos++;

      if (wlist_pos >= MAX_PATH || wlist_pos >= strlen(wlist))
        break;

    }

  }

  return 0;
}


void chreplace(char *s, char c1, char c2)
{
  int i;
  int slen;

  slen = strlen(s);

  for (i = 0; i < slen; i++)
    if (s[i] == c1)
      s[i] = c2;

  return;
}


#define	MAX_MTR	32
int user_match(char *item, char *users)
{
  int i, j;
  int match = 0;
  char mtr[MAX_MTR]; // XXX: ...

  memset(mtr, '\0', sizeof mtr);

  for (i = 0, j = 0; i < strlen(users) + 1; i++) {

    if (users[i] == ',' || i >= strlen(users)) {

      if (!strcmp(mtr, item))
        match++;

      memset(mtr, '\0', sizeof mtr);
      j = 0;

    } else if (j < MAX_MTR) {

      mtr[j++] = users[i];

    }

  }

  return match;
}


int group_match(uid_t uid, char *groups, gid_t pwent_gid)
{
  int i, j;
  char mtr[MAX_MTR]; // XXX: ...

  memset(mtr, '\0', sizeof mtr);

  for (i = 0, j = 0; i < strlen(groups) + 1; i++) {

    if (groups[i] == ',' || i >= strlen(groups)) {

      if (!check_group(get_user(uid), get_gid(mtr)) || get_gid(mtr) == pwent_gid)
        return 1;

      memset(mtr, '\0', sizeof mtr);
      j = 0;

    } else if (j < MAX_MTR) {

      mtr[j++] = groups[i];

    }

  }

  return 0;
}
