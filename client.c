#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define MAXNAME 1024
#define MAXRETRY 10

unsigned int tries=0;
int flag = 1;
void CatchAlarm(int ignored); // Handler for SIGALRM
int ExponentialBackoff(int times);

void main(int argc, char **argv){

  if (argc != 4) {
    printf("Usage: %s <ip> <port> <message>\n", argv[0]);
    exit(0);
  }

  struct itimerval value, ovalue;
  char *ip = argv[1];
  char *msg = argv[3];
  int port = atoi(argv[2]);
  int socket_fd;
  struct sockaddr_in addr;
  char buf[1024];
  int recv_result;
  int res = 0;   
    
  socklen_t addr_size;
  
  struct sigaction handler; // Signal handler
    handler.sa_handler = CatchAlarm;
    if (sigfillset(&handler.sa_mask) < 0) // Block everything in handler
    {
        printf("sigfillset() failed");
        handler.sa_flags = 0;
    }

    if (sigaction(SIGALRM, &handler, 0) < 0)
    {
        printf("sigaction() failed for SIGALRM");
    }

  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  while(tries < MAXRETRY)
  {
	if(flag > 0)
	{
		flag = 0;
  		bzero(buf, 1024);
  		strcpy(buf, msg);
  		sendto(socket_fd, buf, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
  		printf("Data send: %s\n", buf);

		  
	        value.it_value.tv_sec = ExponentialBackoff(tries)/1000000;		
		value.it_value.tv_usec = ExponentialBackoff(tries)%1000000;
		value.it_interval.tv_sec = 0;
		value.it_interval.tv_usec = 0;
		
		printf("interval sec = %d, usec = %d\n", value.it_value.tv_sec, value.it_value.tv_usec);	

  		res = setitimer(ITIMER_REAL, &value, &ovalue); // Set the timeout
		if(res)
		{
			printf("Set timer failed!\n");
		}

  		bzero(buf, 1024);
  		addr_size = sizeof(addr);
  		recv_result = recvfrom(socket_fd, buf, MAXNAME, 0, (struct sockaddr*)&addr, &addr_size);
		
		if(recv_result != -1)
		{
			printf("Data recv: %s\n", buf);
			alarm(0);
			exit(0);			
		}		
	}	
  }
  
  printf("Reach max-retry times, exit program!");
  close(socket_fd);
  exit(1);  
}

void CatchAlarm(int ignored)
{    
    flag = 1;
    tries += 1;
    printf("Time out, retry %d\n", tries);
}


int ExponentialBackoff(int times)
{
	int base = 500000;
	int multiplier = 2;
	int result = 1;
	if(times == 0)
	{
		return base;
	}
	else
	{
		while(times != 0)
		{
			result *= multiplier;
			--times;
		}
		base = base * result;
	}

	if(base > 8000000)
	{
		base = 8000000;
	}
	return base;
}