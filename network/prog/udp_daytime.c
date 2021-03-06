#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define MAXLINE 127

int main(int argc, char *argv[]) {
  struct sockaddr_in servaddr;
  struct hostent *hp;
  //char *sendM;
  char ipaddress[20], buf[MAXLINE +1]="message";
  int s, nbyte, addrlen=sizeof(servaddr);

  if(argc != 3) {
  	printf("Usage: %s ip_address\n", argv[0]);
	exit(0);
  }

  
  hp = gethostbyname(argv[1]);
  if(hp==NULL) {
	printf("gethostbyname fail\n");
	exit(0);
  }

  memcpy(&servaddr.sin_addr, hp->h_addr_list[0],sizeof(servaddr.sin_addr));
  inet_ntop(AF_INET, &servaddr.sin_addr, ipaddress, sizeof(ipaddress));
 
  if((s=socket(PF_INET,SOCK_DGRAM,0)) < 0) {
	perror("socket fail");
	exit(1);
  }

  bzero((char*)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(ipaddress);
  servaddr.sin_port = htons(atoi(argv[2]));
 
/* 
  if(bind(s, (struct sockaddr*)&servaddr, addrlen) <  0) {
	perror("bind fail");
	exit(0);
  }
 */

  
  if(sendto(s,buf,strlen(buf),0,(struct sockaddr*)&servaddr,addrlen) < 0) {
	perror("sendto fail");
	exit(0);
  }

  if((nbyte=recvfrom(s,buf,MAXLINE,0,(struct sockaddr*)&servaddr, &addrlen)) < 0) {
  	perror("recevfrom fail");
	exit(1);
  }

  buf[nbyte] = 0;
  printf("%s", buf);
  close(s);
  return 0;
}
