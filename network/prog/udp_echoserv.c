#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 127

int main(int argc, char* argv[]) {
  struct sockaddr_in servaddr, cliaddr;
  int s, addrlen=sizeof(struct sockaddr), nbyte;
  char buf[MAXLINE+1];

  if(argc != 2) {
   	printf("usage: %s port\n", argv[0]);
	exit(0);
  }

  if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0) {
	perror("sock fail");
	exit(0);
  }

  bzero((char*)&servaddr, addrlen);
  bzero((char*)&cliaddr, addrlen);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  if(bind(s, (struct sockaddr*)&servaddr, addrlen)<0) {
	perror("bind fail");
	exit(0);
  }


  while(1) {
  	puts("Server : waiting request..");
 	nbyte = recvfrom(s,buf,MAXLINE,0,(struct sockaddr*)&cliaddr,&addrlen);
	if(nbyte<0){
		perror("recvfrom fail");
		exit(1);
	}
	buf[nbyte]=0;
      	printf("%d byte recv: %s\n",nbyte, buf);
	if(sendto(s,buf,nbyte,0,(struct sockaddr *)&cliaddr,addrlen) < 0){
		perror("sendto fail");
		exit(1);
	}
	puts("sendto complete");
  }
   
}

