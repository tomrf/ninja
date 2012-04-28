#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "common.h"


uid_t user_name_to_uid(const char *name)
{
    struct passwd   *pwent;

    if ((pwent = getpwnam(name)) == NULL)
        return -1;
    return pwent->pw_uid;
}


gid_t user_group_to_gid(const char *name)
{
  struct group      *grent;

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


int user_get_info(const uid_t uid, uinfo_t *uinfo)
{
    struct passwd   *pwent;
    int             ng = 0;

    sprintf(uinfo->name, "NOUSER");
    uinfo->uid = uid;
    uinfo->gid = -1;
    uinfo->groups = NULL;
    uinfo->ngroups = 0;

    if ((pwent = getpwuid(uid)) == NULL)
        return -1;

    snprintf(uinfo->name, MAX_USERLEN + 1, pwent->pw_name);
    uinfo->uid = pwent->pw_uid;
    uinfo->gid = pwent->pw_gid;
    uinfo->ngroups = 0;

    getgrouplist(uinfo->name, uinfo->gid, NULL, &ng);
    if (ng > 1) {
        uinfo->groups = calloc(ng, sizeof (gid_t));
        getgrouplist(uinfo->name, uinfo->gid, uinfo->groups, &ng);
        uinfo->ngroups = ng;
    }

    return 0;
}


int user_in_group(const uinfo_t *uinfo, const gid_t gid)
{
    size_t     i;

    for (i = 0; i < uinfo->ngroups; i++) {
        if (*(uinfo->groups + i) == gid)
            return 1;
    }

    return 0;
}
