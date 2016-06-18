#pragma once

#include "unp.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <setjmp.h>

#define BUFSIZE 1500

struct rawrec { /* of outgoing UDP data */
    u_short rawrec_seq; /* sequence number */
    u_short rawrec_ttl; /* TTL packet left with */
    struct timeval rawrec_tv; /* time packet left */
};

/* global */
char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];
char strHost[64];

int datalen; /* bytes of data following ICMP header */
char *host;
u_short sport, dport;
int nsent;
pid_t pid;
int waittime, probe, nprobes, nameconvert;
int sendfd, recvfd; /* sent as UDP sock, read on raw ICMP sock */
int ttl, max_ttl;
sigjmp_buf gotalarm;

/* function prototypes */
const char *raw_icmpcode(int);
int raw_recv(int, struct timeval *);
void raw_sig_alrm(int);
void raw_traceloop(void);
void raw_tv_sub(struct timeval *, struct timeval *);

struct raw_proto {
    struct sockaddr *psaSend; /* sockaddr{} for send, from getaddrinfo */
    struct sockaddr *psaRecv; /* sockaddr{} for receiving */
    struct sockaddr *psaLast; /* last sockaddr{} for receiving */
    struct sockaddr *psaBind; /* sockaddr{} for binding source port */
    socklen_t slLen; /* length of sockaddr{}s */
} *prp;
