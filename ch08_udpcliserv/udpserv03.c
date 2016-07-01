#include "unp.h"

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
            len = sizeof(cliaddr);
            n = Recvfrom(udpfd, mesg, MAXLINE, 0, (SA *)&cliaddr, &len);
            Sendto(udpfd, mesg, n, 0, (SA *)&cliaddr, len);
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



