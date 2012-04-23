#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "common.h"

int check_group(const char *user, const gid_t gid)
{
  size_t group_count;
  gid_t gid_list[NGROUPS_MAX+1];
  int i;  

  if (user == NULL)
    return 1;

  initgroups(user, -1);

  group_count = getgroups(0, gid_list);
  getgroups(group_count, gid_list);

  for (i = 0; i < group_count; i++) {
    if (gid_list[i] == gid)
      return 0;
  }

  return 1;
}


char *get_user(uid_t uid)
{
  struct passwd *pwent;

  setpwent();

  while ((pwent = getpwent()) != NULL) {
    if (pwent->pw_uid == uid) {
      endpwent();
      return pwent->pw_name;
    }
  }

  endpwent();

  return NULL;
}


gid_t get_gid(char *name)
{
  struct group *grent;

  setgrent();

  while ((grent = getgrent()) != NULL) {
    if (!strcmp(grent->gr_name, name)) {
      endgrent();
      return grent->gr_gid;
    }
  }

  endgrent();

  return -1;
}

