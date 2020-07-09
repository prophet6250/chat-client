#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/* macros */
#define PORT       9090
#define ALTPORT    10010
#define SERVER_IP "127.0.0.1"
#define FILENAME  "data.txt"

struct Request
{
	char filename[128];
	int port;
};

int main() 
{
	int sock, status, bytes, socklen, altsock, fd;
	struct sockaddr_in server, alt_server;
	struct Request *req;
	char request[256], 
	     response[256];
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("socket() failed\n");
		return sock;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_aton(SERVER_IP, &server.sin_addr);

	socklen = sizeof(server);
	/* don't know what this is for */
	req = (struct Request *)request;
	
	strcpy(req->filename, "server.c");
	req->port = ALTPORT;

	/* send request using normal port and address */
	bytes = sendto(sock, (char *)&request, sizeof(request), 0, 
			(struct sockaddr *)&server, socklen);
	if (bytes < 0) {
		printf("error in sendto()\n");
		return bytes;
	}
	
	altsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	alt_server.sin_family = AF_INET;
	alt_server.sin_addr.s_addr = INADDR_ANY;
	alt_server.sin_port = htons(ALTPORT);
	
	if (bind(altsock, (struct sockaddr *)&alt_server, socklen) < 0) {
		printf("error in bind()\n");
		return -1;
	}

	fd = open(FILENAME, O_RDWR);

	while (1) {
		/* recieve data on alternate port */
		bytes = recvfrom(altsock, response, sizeof(response) - 1, 0, 
				(struct sockaddr *)&alt_server, &socklen);

		if (bytes <= 0) {
			printf("recvfrom() failed\n");
			break;
		} 
		else {
			response[sizeof(response)] = 0;
			printf("%s", response);
			if (strcmp(response, "EOF") == 0)
				break;

			write(fd, response, sizeof(response));
		}
	}
	printf("\n");

	close(fd);
	shutdown(sock, SHUT_RDWR);	
	shutdown(altsock, SHUT_RDWR);
	return 0;
}
