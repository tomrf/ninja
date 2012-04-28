#ifndef _PROTO_H
#define _PROTO_H


/* main.c */
pid_t     get_max_pid(void);   


/* proc.c */
int       proc_get_info(const pid_t pid, pinfo_t *pi);
int       proc_is_root(const pinfo_t *pi);


/* check.c */
int       check_proc(const pinfo_t *pid, const pinfo_t *ppid);


/* user.c */
int       user_get_info(const uid_t uid, uinfo_t *uinfo);
int       user_in_group(const uinfo_t *uinfo, const gid_t gid);
uid_t     user_name_to_uid(const char *name);
gid_t     user_group_to_gid(const char *name);


/* log.c */
void      LOG(const int level, char *fmt, ...);


/* wlist.c */
wlnode_t* wlist_get_ptr(void);
int       wlist_load(const char *path);
int       wlist_match(const uinfo_t *uinfo, const char *path);
void      wlist_debug_dump_list(void);


#endif
