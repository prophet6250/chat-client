#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

#define PORT 9090
#define SERVER_IP "127.0.0.1"

int main() {
	int sock, status, bytes, socklen;
	struct sockaddr_in server;
	char request[256], 
	     response[256];
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("socket() failed\n");
		return sock;
	}

	/* zero out the whole structure */
	memset((void *)&server, 0, sizeof(server));
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	/* use this in favour of inet_addr; see the manpage to know why :) */
	inet_aton(SERVER_IP, &server.sin_addr);

	socklen = sizeof(server);

	while (1) {
		/* what to send? */
		printf("> ");
		fgets(request, 256, stdin);
		bytes = sendto(sock, &request, sizeof(request), 0, 
				(struct sockaddr *)&server, socklen);
		if (bytes < 0) {
			printf("sendto() failed\n");
			break;
		}
		else if (strcmp(request, "bye\n") == 0) {
			printf("closing connection now...\n");
			break;
		}

		bytes = recvfrom(sock, response, sizeof(response), 0, 
				(struct sockaddr *)&server, &socklen);
		if (bytes < 0) {
			printf("recvfrom() failed\n");
			break;
		} 
		else {
			printf("server>: %s", response);
		}
	}
	shutdown(sock, SHUT_RDWR);	
	return 0;
}
