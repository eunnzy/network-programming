#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#define MAXLINE 127

int main(int argc, char * argv[]) {
   struct sockaddr_in servaddr;
   struct hostent *hp; 
   int i, s, nbyte;
   char ipadress[20], buf[MAXLINE+1];

   if(argc != 3) {
	printf("usage: %s ip_address\n",argv[0]);
	exit(0);
   }  

   hp = gethostbyname(argv[1]);
   if(hp==NULL) {
 	printf("gethostbyname fail\n");
	exit(0);
   }
   
   for( i=0; hp->h_addr_list[i]; i++) {  
   memcpy(&servaddr.sin_addr, hp->h_addr_list[i], sizeof(servaddr.sin_addr));
   inet_ntop(AF_INET, &servaddr.sin_addr, ipadress, sizeof(ipadress));
   printf("IP주소 :%s \n", ipadress);
  }

   if((s=socket(PF_INET, SOCK_STREAM, 0)) < 0) {
	perror("sock fail");
	exit(0);
   }

   bzero((char *)&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   inet_pton(AF_INET, ipadress, &servaddr.sin_addr);
   servaddr.sin_port = htons(atoi(argv[2]));

   if(connect(s,(struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
	perror("connect fail");
	exit(0);
   }

   printf("입력: ");
   if(fgets(buf,sizeof(buf), stdin) == NULL)
	exit(0);
   nbyte = strlen(buf);

   if(write(s,buf,nbyte) < 0) {
	printf("write error\n");
	exit(0);
   }

   printf("수신: ");
   if( (nbyte = read(s,buf,MAXLINE)) < 0) {
	perror("read fail");
	exit(0);
   }

   buf[nbyte] = 0;
   printf("%s", buf);
   close(s);
   return 0;
}
