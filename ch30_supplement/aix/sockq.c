#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include <procinfo.h>

#define KMEM "/dev/kmem"
#define FDSINFO fdsinfo64
#define FDSINFOSIZE sizeof(struct FDSINFO)  /* (If we're lucky.) */
#if defined(OPEN_SHRT_MAX)
# if    OPEN_SHRT_MAX<OPEN_MAX
#undef  FDSINFOSIZE             /* (We weren't lucky.) */
#define FDSELEMSIZE (sizeof(struct FDSINFO)/OPEN_SHRT_MAX)
#define FDSINFOSIZE (OPEN_MAX * FDSELEMSIZE)
# endif /* OPEN_SHRT_MAX<OPEN_MAX */
#endif  /* defined(OPEN_SHRT_MAX) */

#define PROCINFO    procentry64
#define PROCSIZE    sizeof(struct PROCINFO)


void err_sys(const char *str)
{
	perror(str);
	exit(1);
}
int Kd; /* KMEM descriptor */

typedef u_longlong_t KA_T; /* kernel memory address type */

extern int procfile(KA_T);

int main(int argc, char *argv[])
{
	pid_t pid;
	static struct FDSINFO *fds;
	static struct procentry64 pinfo;
	KA_T fp; /* pointer */
	unsigned int nf, i;
	int n;


	if (argc != 2) {
		fprintf(stderr, "Usage: %s <PID>\n", basename(argv[0]));
		exit(1);
	}

	if ((Kd = open(KMEM, O_RDONLY, 0)) < 0)
		err_sys("open error");

	pid = (pid_t)atol(argv[1]);
	if ((fds = malloc(FDSINFOSIZE)) == NULL)
		err_sys("malloc error");

	if ((n = getprocs64(&pinfo, PROCSIZE, fds, FDSINFOSIZE, &pid, 1)) != 1) {
		if (n < 0)
			err_sys("getprocs64 error");
		else {
			fprintf(stderr, "Process table is empty\n");
			exit(1);
		}
	}

	nf = pinfo.pi_maxofile;

	for (i = 0; i < nf; i++) {
		fp = fds->pi_ufd[i].fp;
		procfile(fp);
	}

	free(fds);
	return 0;
}






