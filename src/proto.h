#include "proc.h"

/* main.c */
int main(int argc, char **argv);
void help(void);

/* mloop.c */
void main_loop(int mgroup, int cycledelay, char *wlist);
int* get_active_pids(void);

/* user.c */
int check_group(const char *user, const gid_t gid);
char* get_user(uid_t uid);
gid_t get_gid(char *name);

/* proc.c */
int get_proc_info(const pid_t pid, struct proc_info *pi);

/* log.c */
void LOG(char *fmt, ...);
void die(char *fmt, ...);
char* timestamp(void);
char* replace(char *string, char c1, char c2);

/* wlist.c */
off_t fd_len(int fd);
char* wlist_read(char *path);
int wlist_match(char *wlist, pid_t pid, uid_t uid, gid_t gid);
char* get_map_name(pid_t pid);
void chreplace(char *s, char c1, char c2);
int user_match(char *item, char *users);
int group_match(uid_t uid, char *groups, gid_t pwent_gid);
char* get_map_exec_offset(void);

/* signal.c */
void sighandler(int s);

/* config.c */
int conf_read(const char *path);
void stolower(char *s);

/* file.c */
int file_isregular(char *fpath);
long file_size(char *fpath);
