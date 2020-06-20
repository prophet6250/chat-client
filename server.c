#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define PORT 9090

int main() {
	int sock, status, socklen, bytes;
	char response[256], 
	     request[256];
	struct sockaddr_in host;
	
	/* make a socket */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("socket() failed\n");
		return sock;
	}

	/* zero out the whole strucutre */
	memset((void *)&host, 0, sizeof(host));

	/* socket options in a structure */
	host.sin_family=  AF_INET;
	host.sin_addr.s_addr = INADDR_ANY;
	host.sin_port = htons(PORT);

	socklen = sizeof(host);
	/* bind the socket structure to the socket we created */
	if (status = bind(sock, (struct sockaddr *)&host, socklen) < 0) {
		printf("bind() failed\n");
		return status;
	}
	
	while (1) {
		bytes = recvfrom(sock, request, sizeof(request), 0, 
				(struct sockaddr *)&host, &socklen);
		if (bytes < 0) {
			printf("recvfrom() failed\n");
			break;
		}

		printf("recieved: %s\n", request);
		if (strcmp(request, "bye\n") == 0) {
			printf("closing connection now...\n");
			break;
		}
		
		/* send the request as a response to the client */
		memmove(response, request, sizeof(request));

		bytes = sendto(sock, &response, sizeof(response), 0, 
				(struct sockaddr *)&host, socklen);
		if (bytes < 0) {
			printf("sendto() failed\n");
			break;
		}
	}
	shutdown(sock, SHUT_RDWR);
	return 0;
}

