#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 64
#define MOD 1000

void logexit(const char *str)
{
	if(errno) perror(str);
	else puts(str);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	uint32_t counter = 0;
	char buffer[BUFSZ];
	struct sockaddr_in clt, srv = {
		.sin_family = AF_INET,
		.sin_port = htons(51515),
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK)
	};

	// Creating socket and binding
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) logexit("socket");
	if (bind(sock, (struct sockaddr *) &srv, sizeof(srv)) < 0) logexit("binding");

	// Forever looping. Waiting for clients.
	while (1) {
		int addrlen = sizeof(clt);
		listen(sock, 1);

		// New client. Accepting.
		int newsock = accept(sock, (struct sockaddr *) &clt, &addrlen);
		if (newsock == -1) logexit("accept");

		// Receiving new data
		if (1 != recv(newsock, buffer, 1, MSG_WAITALL)) logexit("recv");

		int tmp = (int)counter;
		if (buffer[0] == '+')
			tmp++;
		else if (buffer[0] == '-') {
			tmp--;

			if (tmp < 0) tmp += 1000;
		}
		else
			logexit("invalid value");
		tmp %= MOD;

		// Sending counter value to confirm.
		uint32_t value = htonl(tmp);
		send(newsock, &value, 4, 0);

		// Setting timer for 1s0ms.
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(newsock, &fdset);	
		struct timeval t = { .tv_sec = 1, .tv_usec = 0 };
		setsockopt(newsock, SOL_SOCKET, SO_RCVTIMEO, 
				(struct timeval *)&t, sizeof(struct timeval));

		if (0 == select(newsock + 1, &fdset, NULL, NULL, &t)) {
			fprintf(stdout, "T\n");
		}
		else  {
			// Receiving counter confirmation.
			if (3 != recv(newsock, buffer, 3, MSG_WAITALL)) logexit("recv");

			// Checking value received and printing.
			value = atoi(buffer);
			if (tmp == value) {
				counter = tmp;
				fprintf(stdout, "%d\n", counter);
			}

		}
		close(newsock);
	}
	close(sock);

	exit(EXIT_SUCCESS);
}
