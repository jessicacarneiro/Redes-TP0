#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

void logexit(const char *str)
{
	if(errno) perror(str);
	else puts(str);
	exit(EXIT_FAILURE);
}

void int2Char(int value, char *out)
{
}

int main(int argc, char **argv)
{
	char buffer[4];
	char ch_counter[4];
	int counter;

	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1)
		logexit("socket");

	struct sockaddr_in clt_addr;
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(51515),
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK)
	};

	if (bind(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		logexit("binding");

	listen(s, SOMAXCONN);
	int clt_addr_len = sizeof(clt_addr);
	int new_s = accept(s, (struct sockaddr *) &clt_addr, &clt_addr_len);

	if (new_s == -1)
		logexit("accepts");

	fd_set fdset;
	FD_SET(new_s, &fdset);	
	struct timeval t = { .tv_sec = 1 }; // 1s
	setsockopt(new_s,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&t,sizeof(struct timeval));
	
	int n = recv(new_s,buffer,1,0);

	if (n > 0)
		fprintf(stdout, "I've received %c\n", buffer[0]);

	send(new_s,buffer,3,0);
	n = recv(new_s,buffer,3,0);

	close(new_s);
	close(s);

	exit(EXIT_SUCCESS);
}

