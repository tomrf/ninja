#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <linux/connector.h>
#include <linux/netlink.h>
#include <linux/cn_proc.h>

#define SEND_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op)))
#define RECV_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(struct proc_event)))

#define SEND_MESSAGE_SIZE    (NLMSG_SPACE(SEND_MESSAGE_LEN))
#define RECV_MESSAGE_SIZE    (NLMSG_SPACE(RECV_MESSAGE_LEN))

#define max(x,y) ((y)<(x)?(x):(y))
#define min(x,y) ((y)>(x)?(x):(y))

#define BUFF_SIZE (max(max(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE), 1024))
#define MIN_RECV_SIZE (min(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE))

#define PROC_CN_MCAST_LISTEN (1)
#define PROC_CN_MCAST_IGNORE (2)


void handle_msg (struct cn_msg *cn_hdr)
{
    char cmdline[1024], fname1[1024], ids[1024], fname2[1024], buf[1024];
    int r = 0, fd, i;
    FILE *f = NULL;
    struct proc_event *ev = (struct proc_event *)cn_hdr->data;

    snprintf(fname1, sizeof(fname1), "/proc/%d/status", ev->event_data.exec.process_pid);
    snprintf(fname2, sizeof(fname2), "/proc/%d/cmdline", ev->event_data.exec.process_pid);

    f = fopen(fname1, "r");
    fd = open(fname2, O_RDONLY);

    memset(&cmdline, 0, sizeof(cmdline));
    memset(&ids, 0, sizeof(ids));

    while (f && fgets(buf, sizeof(buf), f) != NULL) {
        if (strstr(buf, "Uid")) {
            strtok(buf, "\n");
            snprintf(ids, sizeof(ids), "%s", buf);
        }
    }
    if (f)
        fclose(f);

    if (fd > 0) {
        r = read(fd, cmdline, sizeof(cmdline));
        close(fd);

        for (i = 0; r > 0 && i < r; ++i) {
            if (cmdline[i] == 0)
                cmdline[i] = ' ';
        }
    }

    switch(ev->what){
    case PROC_EVENT_FORK:
        printf("FORK:parent(pid,tgid)=%d,%d\tchild(pid,tgid)=%d,%d\t[%s]\n",
               ev->event_data.fork.parent_pid,
               ev->event_data.fork.parent_tgid,
               ev->event_data.fork.child_pid,
               ev->event_data.fork.child_tgid, cmdline);
        break;
    case PROC_EVENT_EXEC:
        printf("EXEC:pid=%d,tgid=%d\t[%s]\t[%s]\n",
               ev->event_data.exec.process_pid,
               ev->event_data.exec.process_tgid, ids, cmdline);
        break;
    case PROC_EVENT_EXIT:
        printf("EXIT:pid=%d,%d\texit code=%d\n",
               ev->event_data.exit.process_pid,
               ev->event_data.exit.process_tgid,
               ev->event_data.exit.exit_code);
        break;
    case PROC_EVENT_UID:
        printf("UID:pid=%d,%d ruid=%d,euid=%d\n",
            ev->event_data.id.process_pid, ev->event_data.id.process_tgid,
            ev->event_data.id.r.ruid, ev->event_data.id.e.euid);
        break;
    default:
        break;
    }
}


int main(int argc, char **argv)
{
    int             sk_nl;
    int             err;
    socklen_t       from_nla_len;
    char            buff[BUFF_SIZE];
    int             rc = -1;
    struct nlmsghdr *nl_hdr;
    struct cn_msg   *cn_hdr;
    size_t          recv_len = 0;
    struct sockaddr_nl my_nla, kern_nla, from_nla;
    enum proc_cn_mcast_op *mcop_msg;

    setvbuf(stdout, NULL, _IONBF, 0);

    sk_nl = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (sk_nl == -1) {
        printf("socket sk_nl error");
        return rc;
    }

    my_nla.nl_family = AF_NETLINK;
    my_nla.nl_groups = CN_IDX_PROC;
    my_nla.nl_pid = getpid();

    kern_nla.nl_family = AF_NETLINK;
    kern_nla.nl_groups = CN_IDX_PROC;
    kern_nla.nl_pid = 1;

    err = bind(sk_nl, (struct sockaddr *)&my_nla, sizeof(my_nla));
    if (err == -1) {
        printf("binding sk_nl error");
        goto close_and_exit;
    }
    nl_hdr = (struct nlmsghdr *)buff;
    cn_hdr = (struct cn_msg *)NLMSG_DATA(nl_hdr);
    mcop_msg = (enum proc_cn_mcast_op*)&cn_hdr->data[0];

    printf("sending proc connector: PROC_CN_MCAST_LISTEN... ");
    memset(buff, 0, sizeof(buff));
    *mcop_msg = PROC_CN_MCAST_LISTEN;

    nl_hdr->nlmsg_len = SEND_MESSAGE_LEN;
    nl_hdr->nlmsg_type = NLMSG_DONE;
    nl_hdr->nlmsg_flags = 0;
    nl_hdr->nlmsg_seq = 0;
    nl_hdr->nlmsg_pid = getpid();

    cn_hdr->id.idx = CN_IDX_PROC;
    cn_hdr->id.val = CN_VAL_PROC;
    cn_hdr->seq = 0;
    cn_hdr->ack = 0;
    cn_hdr->len = sizeof(enum proc_cn_mcast_op);

    if (send(sk_nl, nl_hdr, nl_hdr->nlmsg_len, 0) != nl_hdr->nlmsg_len) {
        printf("failed to send proc connector mcast ctl op!\n");
        goto close_and_exit;
    }

    printf("sent\n");

    if (*mcop_msg == PROC_CN_MCAST_IGNORE) {
        rc = 0;
        goto close_and_exit;
    }

    printf("Reading process events from proc connector.\n");

    for(memset(buff, 0, sizeof(buff)), from_nla_len = sizeof(from_nla);
      ; memset(buff, 0, sizeof(buff)), from_nla_len = sizeof(from_nla)) {

        struct nlmsghdr *nlh = (struct nlmsghdr*)buff;

        memcpy(&from_nla, &kern_nla, sizeof(from_nla));

        recv_len = recvfrom(sk_nl, buff, BUFF_SIZE, 0,
                (struct sockaddr*)&from_nla, &from_nla_len);

        if (from_nla.nl_pid != 0)
            continue;
        if (recv_len < 1)
            continue;

        while (NLMSG_OK(nlh, recv_len)) {
            cn_hdr = NLMSG_DATA(nlh);
            if (nlh->nlmsg_type == NLMSG_NOOP)
                continue;
            if ((nlh->nlmsg_type == NLMSG_ERROR) ||
                (nlh->nlmsg_type == NLMSG_OVERRUN))
                break;
            handle_msg(cn_hdr);
            if (nlh->nlmsg_type == NLMSG_DONE)
                break;
            nlh = NLMSG_NEXT(nlh, recv_len);
        }

    }

close_and_exit:
    close(sk_nl);
    exit(rc);

    return 0;
}

