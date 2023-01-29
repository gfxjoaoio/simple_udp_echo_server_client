#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXNAME 1024

extern int errno;
void main(int argc, char **argv)
{
	int socket_fd;
	int recfd; 
	int length;
	int nbytes;
	int port = atoi(argv[1]);
	char buf[BUFSIZ];
	struct sockaddr_in server_addr; /* address of this service */
	struct sockaddr_in client_addr; /* address of client    */
	
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
			perror ("socket failed");
			exit(EXIT_FAILURE);
	}
	
	memset(&server_addr, '\0', sizeof(server_addr));	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
			perror ("bind failed\n");
			exit(1);
	}

	length = sizeof(client_addr);
	printf("Server is ready to receive !!\n");
	printf("Can strike Cntrl-c to stop Server >>\n");
	while (1) {
			if ((nbytes = recvfrom(socket_fd, &buf, MAXNAME, 0, (struct sockaddr*)&client_addr, (socklen_t *)&length)) <0) {
					perror ("could not read datagram!!");
					continue;
			}


			printf("Received data form %s : %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
			printf("%s\n", buf);

			/* return to client */
			if (sendto(socket_fd, &buf, nbytes, 0, (struct sockaddr*)&client_addr, length) < 0) {
					perror("Could not send datagram!!\n");
					continue;
			}
			printf("Can Strike Crtl-c to stop Server >>\n");
	}
}