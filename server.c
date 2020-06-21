/* for socket functions */
#include <sys/socket.h>
/* for wait functions */
#include <sys/wait.h>
/* for child process creation and termination signal commands */
#include <signal.h>
/* for conversions from host to network bytes */
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9090

void handleSignal() {
	int stat;
	/* wait until the status of the terminated child is recieved */
	/* -1 means status is awaited for all child processes */
	while (waitpid(-1, &stat, WNOHANG) > 0);
}

int main() {
	int sock, status, socklen, bytes;
	char response[256], 
	     request[256];
	struct sockaddr_in client;
	pid_t pid;

	/* handle the SIGCHILD signal by using the handleSignal() routine */
	signal(SIGCHLD, handleSignal);	
	
	/* make a socket */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("socket() failed\n");
		return sock;
	}

	/* zero out the whole strucutre */
	memset((void *)&client, 0, sizeof(client));

	/* socket options in a structure */
	client.sin_family=  AF_INET;
	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons(PORT);

	socklen = sizeof(client);
	/* bind the socket structure to the socket we created */
	if (status = bind(sock, (struct sockaddr *)&client, socklen) < 0) {
		printf("bind() failed\n");
		return status;
	}
	
	while (1) {
		/* recieve the message in the buffer "request" */
		bytes = recvfrom(sock, request, sizeof(request), 0, 
				(struct sockaddr *)&client, &socklen);
		if (bytes < 0) {
			printf("recvfrom() failed\n");
			break;
		}
		if (strcmp(request, "bye\n") == 0) {
			printf("closing connection from %s\n");
			continue;
		}	
		
		pid = fork();
		/* if 0, then we're inside a child process, else we're in the
		 * parent process */
		if (pid == 0) {
			printf("request: %s\n", request);
			/* this is to get more requests, and to execute them
			 * faster */
			sleep(2);
			
			/* send the request as a response to the client */
			memmove(response, request, sizeof(request));

			/* send respose to the client in a child process */
			bytes = sendto(sock, &response, sizeof(response), 0,
				(struct sockaddr *)&client, socklen);
			if (bytes < 0) {
				printf("sendto() failed\n");
				return bytes;
			}
			/* exit the child process */
			/* this child process will now be handled according to
			 * the handleSignal() routine we described in the
			 * beginning */
			exit(0);
		}
	}
	/* close this socket, so no further operations may be done using this
	 * socket */
	shutdown(sock, SHUT_RDWR);
	return 0;
}

