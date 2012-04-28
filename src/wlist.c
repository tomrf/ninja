#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

static int  wlist_add(const char *path, uid_t *uids, size_t nuids, gid_t *gids, size_t ngids);
static int  wlist_parse_line(const char *line);
//static void wlist_destroy(void);

void wlist_debug_dump_list(void)
{
    wlnode_t   *ptr;
    size_t     i;

    ptr = wlist_get_ptr();

    for (;ptr != NULL; ptr = ptr->next) {
        if (!*(ptr->path))
            continue;

        printf("+ %p\n", ptr);
        printf("  path: %s\n", ptr->path);
        printf("  uids (%d): ", ptr->nuids);
        for (i = 0; i < ptr->nuids; i++)
            printf("%d ", *(ptr->uids + i));
        printf("\n  gids (%d): ", ptr->ngids);
        for (i = 0; i < ptr->ngids; i++)
            printf("%d ", *(ptr->gids + i));
        printf("\n");
    }
}


wlnode_t* wlist_get_ptr(void)
{
    static wlnode_t    *ptr = NULL;

    if (ptr == NULL) {
        ptr = malloc(sizeof (wlnode_t));
        /* xxx: handle error */
        ptr->next = NULL;
        ptr->path[0] = '\0';
    }

    return ptr;
}


//static void wlist_destroy(void) {
/* xxx: wlist_destroy(), remember to free uids, gids */
//}


static int wlist_add(const char *path, uid_t *uids, size_t nuids, gid_t *gids, size_t ngids)
{
    wlnode_t    *ptr, *new;

    ptr = wlist_get_ptr();

    if (ptr->path[0] == '\0') {
        new = ptr;
    } else {
        new = malloc(sizeof (wlnode_t));
        /* xxx: handle error */
        for (; ptr->next != NULL; ptr = ptr->next) ;
        ptr->next = new;
    }

    snprintf(new->path, PATH_MAX, "%s", path);
    new->uids = uids;
    new->gids = gids;
    new->nuids = nuids;
    new->ngids = ngids;
    new->next = NULL;

    return 0;
}


int wlist_load(const char *path)
{
    FILE     *fp;
    char     *buf;
    size_t   len = 0, i = 0;
    int      c;

    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            if (i > len) len = i;
            i = 0;
        }
        else i++;
    }

    rewind(fp);

    len += 2;
    buf = malloc(len);
    /* xxx: handle error */

    while (fgets(buf, len, fp) != NULL)
        wlist_parse_line(buf);

    fclose(fp);
    free(buf);

    return 0;
}


static void chreplace(char *s, char c1, char c2)
{
    while (*s) {
        if (*s == c1) *s = c2;
        (void) *s++;
    }
}


static int wlist_parse_line(const char *line)
{
    char      *buf, *tmp, *ptr;
    char      path[PATH_MAX];
    size_t    len;
    uid_t     *uids = NULL;
    gid_t     *gids = NULL;
    int       i = 0;
    int       cat = 0;
    int       num = 0;
    int       gids_size = 10, uids_size = 10;
    int       ngids = 0, nuids = 0;

    len = strlen(line) + 1;
    buf = malloc(len);
    tmp = malloc(len);

    uids = malloc(uids_size * sizeof (uid_t));
    gids = malloc(gids_size * sizeof (gid_t));

    /* xxx: handle error */

    snprintf(buf, len, "%s", line);
    chreplace(buf, '\n', '\0');
    chreplace(buf, '\r', '\0');

    printf("parsing [%s]\n", buf);

    memset(tmp, '\0', len);

    for (ptr = buf; ; (void) *ptr++) {
        if (*ptr != '\0' && *ptr != ',' && *ptr != ':') {
            *(tmp + i++) = *ptr;
            continue;
        }
        if (strlen(tmp) > 0) {
            if (cat == 0)
                snprintf(path, sizeof path, "%s", tmp);
            if (cat == 1) {
                if (isdigit(*tmp)) *(gids + num++) = atoi(tmp);
                else *(gids + num++) = user_group_to_gid(tmp);;
                if (num == gids_size) {
                    gids_size += 10;
                    gids = realloc(gids, gids_size * sizeof (gid_t));
                }
                ngids++;
            }
            if (cat == 2) {
                if (isdigit(*tmp)) *(uids + num++) = atoi(tmp);
                else *(uids + num++) = user_name_to_uid(tmp);
                if (num == uids_size) {
                    uids_size += 10;
                    uids = realloc(uids, uids_size * sizeof (uid_t));
                }
                nuids++;
            }
        }
        if (*ptr == ':') {
            cat++;
            num = 0;
        }
        if (*ptr == '\0')
            break;
        memset(tmp, '\0', len);
        i = 0;
    }

    wlist_add(path, uids, nuids, gids, ngids);

    return 0;
}


int wlist_match(const uinfo_t *uinfo, const char *path)
{
    wlnode_t   *ptr;
    size_t     i, j;

    ptr = wlist_get_ptr();

    for (; ptr != NULL; ptr = ptr->next) {
        if (!strcmp(ptr->path, path)) {
            for (i = 0; i < ptr->nuids; i++) {
                if (uinfo->uid == *(ptr->uids + i))
                    return 1;
            }
            for (i = 0; i < ptr->ngids; i++) {
                for (j = 0; j < uinfo->ngroups; j++) {
                    if (*(uinfo->groups + j) == *(ptr->gids + i))
                        return 2;
                }
            }
        }
    }

    return 0;
}
