#include "unp.h"

int
main(int argc, char **argv)
{
	int		sockfd;
	struct sockaddr_in servaddr;

	if (argc != 2)
		err_quit("usage: %s <IPaddress>", *argv);

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof servaddr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) & servaddr, sizeof servaddr);

    if (freopen("./bin.txt", "r", stdin) == NULL)
        err_sys("freopen error");
	str_cli(stdin, sockfd);	/* do it all */

	exit(0);
}
