#include "unp.h"
#include <sys/socket.h>

#define CONTROLLEN CMSG_LEN(sizeof(struct in_addr))
static struct cmsghdr *cmptr = NULL;

static void sig_chld(int);

int main(int argc, char **argv)
{
    int listenfd, connfd, udpfd, nready, maxfd;
    char mesg[MAXLINE];
    pid_t childpid;
    fd_set rset;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    const int on = 1;
    ssize_t n;

    struct iovec iov[1];
    struct msghdr msg;
    struct in_addr ia;
    struct cmsghdr *curptr;
    char *ptr;

    if ((cmptr = malloc(CONTROLLEN)) == NULL)
        err_sys("malloc error");

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    /* create UDP socket */
    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);
    Setsockopt(udpfd, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(udpfd, (SA *)&servaddr, sizeof(servaddr));

    Signal(SIGCHLD, sig_chld);

    maxfd = listenfd > udpfd ? listenfd : udpfd;
    
    for (;;) {
        FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        if ((nready = select(maxfd+1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
                continue;
            err_sys("select error");
        }
        if (FD_ISSET(listenfd, &rset)) {
            len = sizeof(cliaddr);
            connfd = Accept(listenfd, (SA *)&cliaddr, &len);

            if ((childpid = fork()) == 0) {
                Close(listenfd);
                str_echo(connfd);
                exit(0);
            }
            Close(connfd);
        }
        if (FD_ISSET(udpfd, &rset)) {
            iov[0].iov_base = mesg;
            iov[0].iov_len = sizeof(mesg);
            msg.msg_iov = iov;
            msg.msg_iovlen = 1;
            msg.msg_name = (SA *)&cliaddr;
            msg.msg_namelen = sizeof(cliaddr);
            msg.msg_control = cmptr;
            msg.msg_controllen = CONTROLLEN;

            n = recvmsg(udpfd, &msg, 0);
            if (n == -1)
                err_sys("recvmsg error");
            printf("received %zd bytes from %s, ", n, Sock_ntop((SA *)&cliaddr, sizeof(cliaddr)));
            for (curptr = CMSG_FIRSTHDR(&msg); curptr != NULL;
                    curptr = CMSG_NXTHDR(&msg, curptr)) {
                memcpy(&ia, CMSG_DATA(curptr), sizeof(ia));
                if ((ptr = inet_ntoa(ia)) == NULL)
                    err_sys("inet_ntoa error");
                printf( "to %s\n", ptr);
            }
            Sendto(udpfd, mesg, n, 0, (SA *)&cliaddr, sizeof(cliaddr));
        }
    }
}

static void sig_chld(int signo)
{
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
        printf("child %ld terminated\n", (long)pid);

    return;

}



