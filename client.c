#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 64


void logexit(const char *str)
{
        if(errno) perror(str);
        else puts(str);
        exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
        if(argc < 2 || (argv[1][0] != '+' && argv[1][0] != '-'))
                logexit("invalid parameter");

        int s = socket(AF_INET, SOCK_STREAM, 0);
        if(s == -1) logexit("socket");

        struct in_addr addr = { .s_addr = htonl(INADDR_LOOPBACK) };
        struct sockaddr_in dst = { .sin_family = AF_INET,
                                   .sin_port = htons(51515),
                                   .sin_addr = addr };
        struct sockaddr *sa_dst = (struct sockaddr *)&dst;

        if(connect(s, sa_dst, sizeof(dst))) logexit("connect");

        send(s, argv[1], 1, 0);
        char buf[BUFSZ];
        if(4 != recv(s, buf, 4, MSG_WAITALL)) logexit("recv");
        uint32_t counter = ntohl(*(uint32_t *)buf);
        snprintf(buf, BUFSZ, "%03d", counter);
        send(s, buf, 3, 0);
	printf("%d\n", counter);

        close(s);
        exit(EXIT_SUCCESS);
}
