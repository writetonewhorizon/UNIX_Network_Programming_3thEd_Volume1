#include "udpcksum-ex.h"
#include <libgen.h>

struct sockaddr *dest, *local;
socklen_t destlen, locallen;
int cksum;
char *device;
struct sockaddr localaddr; /* must be global variable */
int linktype;


int main(int argc, char *argv[])
{
    struct addrinfo *paiRes, aiHints;
    int ret;

    if (argc != 5)
        err_quit("Usage: %s <host> <service> <device> <needCksum>", basename(argv[0]));

    aiHints.ai_flags = AI_CANONNAME;
    aiHints.ai_family = AF_INET;
    aiHints.ai_socktype = SOCK_DGRAM;
    aiHints.ai_protocol = IPPROTO_UDP;

    if ((ret = getaddrinfo(argv[1], argv[2], &aiHints, &paiRes)) != 0)
        err_quit("getaddrinfo error: %s", gai_strerror(ret));
    device = argv[3];
    cksum = atoi(argv[4]);
    char strHost[64];
    int port;
    if (paiRes->ai_family != AF_INET)
        err_quit("ai_family error");
    if (inet_ntop(paiRes->ai_family, &((struct sockaddr_in *)paiRes->ai_addr)->sin_addr, strHost, sizeof(strHost)) == NULL)
        err_sys("inet_ntop error");
    port = ntohs(((struct sockaddr_in *)paiRes->ai_addr)->sin_port);

    printf("Query to %s:%d\n", strHost, port);

    dest = paiRes->ai_addr;
    destlen = paiRes->ai_addrlen;
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        err_sys("socket error");
    if (connect(sockfd, dest, destlen) < 0)
        err_sys("connect error");

    locallen = sizeof(struct sockaddr);
    bzero(&localaddr, locallen);
    if (getsockname(sockfd, &localaddr, &locallen) < 0)
        err_sys("getsockname error");
    if (((struct sockaddr_in *)&localaddr)->sin_addr.s_addr == htonl(INADDR_ANY))
        err_quit("can't determine local address");
    local = &localaddr;
    if (inet_ntop(AF_INET, &((struct sockaddr_in *)local)->sin_addr, strHost, sizeof(strHost)) == NULL)
        err_sys("inet_ntop error");
    port = ntohs(((struct sockaddr_in *)local)->sin_port);
    printf("From %s:%d\n", strHost, port);


    /* open raw socket */
    dlt_open_raw();
    dlt_open_pcap(); 
    dlt_test_udp(); /* step 1: dlt_send_dnsquery() -> dlt_udp_write(),
                       step 2: dlt_udp_read(),
                       step 3: check UDP checksum */
    
    return 0;
}



