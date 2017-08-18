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
	uint32_t counter = 0, tmp = 0;
	int sock, newsock, addrlen, nrecv;
	char buffer[BUFSZ], rcv[BUFSZ];
	struct sockaddr_in clt;
	struct sockaddr_in srv = {
		.sin_family = AF_INET,
		.sin_port = htons(51515),
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK)
	};

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		logexit("socket");

	if (bind(sock, (struct sockaddr *) &srv, sizeof(srv)) < 0)
		logexit("binding");

	while (1) {
		listen(sock, 1);

		addrlen = sizeof(clt);
		newsock = accept(sock, (struct sockaddr *) &clt, &addrlen);
		if (newsock == -1)
			logexit("accept");

		fd_set fdset;
		FD_SET(newsock, &fdset);	
		struct timeval t = { .tv_sec = 1 }; // 1s
		setsockopt(newsock,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&t,sizeof(struct timeval));

		memset(buffer, 0, sizeof(buffer));

		if (1 != recv(newsock, buffer, 1, MSG_WAITALL))
			logexit("recv");

		if (buffer[0] == '+')
			tmp = (counter + 1) % 1000;
		else if (buffer[0] == '-')
			tmp = (counter - 1) % 1000;
		else
			logexit("invalid value");

		uint32_t value = htonl(tmp);
		send(newsock, &value, 4, 0);

		if (3 != recv(newsock, rcv, 3, MSG_WAITALL))
			logexit("recv");

		snprintf(buffer, BUFSZ, "%03d", tmp);
		if (memcmp(buffer, rcv, 3) == 0)
			counter = tmp;

		close(newsock);
	}
	close(sock);

	exit(EXIT_SUCCESS);
}

