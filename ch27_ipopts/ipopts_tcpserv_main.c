#include	"unp.h"
#include <libgen.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#define OPTLEN 44

extern void ipopts_str_echo(int);
extern void ipopts_inet_srcrt_print(u_char *, int);
static void	sig_chld(int signo)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
        printf("child %u terminated\n", pid);
    return;
}

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

    if (argc != 2)
        err_quit("Usage: %s <Port>", basename(argv[0]));

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */
    u_char ptr[OPTLEN];
    int len = OPTLEN;

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ((connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}

        if (getsockopt(connfd, IPPROTO_IP, IP_OPTIONS, ptr, (socklen_t *)&len) < 0)
            err_sys("getsockopt error");
        if (len > 0) {
            printf("receiced IP options, len = %d\n", len);
            ipopts_inet_srcrt_print(ptr, len);
#ifdef _SERVER_CLEAN_SSR
            if (setsockopt(connfd, IPPROTO_IP, IP_OPTIONS, NULL, 0) < 0)
                err_sys("setsockopt clean IP_OPTIONS error");
#endif
        }

		if ((childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			ipopts_str_echo(connfd);	/* process the request */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
