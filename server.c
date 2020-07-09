#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 9090

struct Request 
{
	char filename[128];
	int port;
}; 

void handleSignal() 
{
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0);
}

int main()
{
	int sock, status, socklen, bytes, fd, altsock;
	char response[256], 
	     buf[256];
	struct sockaddr_in client;
	pid_t pid;
	struct Request *req;

	signal(SIGCHLD, handleSignal);	
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("socket() failed\n");
		return sock;
	}

	memset((void *)&client, 0, sizeof(client));

	client.sin_family=  AF_INET;
	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons(PORT);

	socklen = sizeof(client);
	if (status = bind(sock, (struct sockaddr *)&client, socklen) < 0) {
		printf("bind() failed\n");
		return status;
	}
	
	bytes = 0;
	while (1) {
		bytes = recvfrom(sock, buf, sizeof(buf), 0, 
				(struct sockaddr *)&client, &socklen);

		if (bytes < 0) {
			printf("recvfrom() failed\n");
			break;
		}

		pid = fork();
		if (pid == 0) {
			/* sock will only be used to listen for connections,
			 * altsock will be used for data transfer */
			close(sock);

			/* buf was recieved as a char* from client side.
			 * check client side code for more details */
			req = (struct Request *)buf;
			printf("%s> requested %s : client port: %d\n", 
					inet_ntoa(client.sin_addr), 
					req->filename, req->port);

			/* now I'll prefer open() to fopen() forever x( */
			fd = open(req->filename, O_RDONLY);
			if (fd == -1) {
				printf("failed to open %s: file doesn't exist\n",
				        req->filename);
				shutdown(sock, SHUT_RDWR);
				return fd;
			}

			altsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (altsock < 0) {
				printf("socket() failed for alternate socket\n");
				return altsock;
			}
			
			/* send data to a new client port, as request by the
			 * client */
			client.sin_port = htons(req->port);
			
			while (1) {
				memset(response, 0, sizeof(response));
				/* file descriptors make lyf easier :) */
				bytes = read(fd, &response, sizeof(response) - 1);

				if (bytes <= 0) {
					strcpy(response, "EOF");
				}

				bytes = sendto(altsock, &response, 
						sizeof(response), 0, 
						(struct sockaddr *)&client, 
						socklen);

				printf("server> bytes sent= %d", bytes);

				if (bytes < 0) {
					printf("error in sending response\n");
					close(altsock);
					return bytes;
				}

				if (strcmp(response, "EOF") == 0)
					break;
			}
			shutdown(altsock, SHUT_RDWR);
			exit(0);
		}
	}
	shutdown(sock, SHUT_RDWR);
	return 0;
}

