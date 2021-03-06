#include "mynames.h"
#include <sys/time.h>
#include <setjmp.h>

#define ITIMER_REAL      0
#define ITIMER_VIRTUAL   1
#define ITIMER_PROF      2

static sigjmp_buf jmpbuf;

static void 
sig_alarm(int signo)
{
	siglongjmp(jmpbuf, 1);
}

int 
main(int argc, char **argv)
{
	char		host      [MAXLINE], serv[32];
	char           *ptr;
	struct sockaddr_in *saptr;
	int		ret;
	struct itimerval delay;

	saptr = malloc(sizeof(struct sockaddr_in));
	if (saptr == NULL)
		err_sys("malloc error");

	if (signal(SIGALRM, sig_alarm) == SIG_ERR)
		err_sys("signal error");
	delay.it_value.tv_sec = 0;
    if (getenv("TIMEO") == NULL)
        err_quit("unset TIMEO");
	if ((delay.it_value.tv_usec = atoi(getenv("TIMEO"))) < 0)
        err_quit("getenv TIMEO error");
	delay.it_interval.tv_sec = 0;
	delay.it_interval.tv_usec = 0;
	ret = setitimer(ITIMER_REAL, &delay, NULL);
	if (ret)
		err_sys("setitimer error");

	if (sigsetjmp(jmpbuf, 1) == 0) {
		while (--argc > 0) {
			ptr = *++argv;
			bzero(saptr, sizeof(struct sockaddr_in));
			saptr->sin_family = AF_INET;
			if (inet_aton(ptr, &saptr->sin_addr) == 0) {
				err_msg("inet_aton error: %s", ptr);
				continue;
			}
			ret = getnameinfo((SA *) saptr, sizeof(struct sockaddr_in),
					  host, sizeof(host),
					  serv, 0, 0);
			if (ret != 0) {
				err_msg("getnameinfo error for host: %s: %s", ptr, gai_strerror(ret));
				continue;
			}
			printf("official hostname: %s\n", host);
		}
	} else {
		printf("timed out\n");
        ptr = inet_ntop(AF_INET, &saptr->sin_addr, host, MAXLINE);
        if (ptr != NULL)
            printf("address: %s\n", ptr);
	}

	free(saptr);
	exit(0);

}
