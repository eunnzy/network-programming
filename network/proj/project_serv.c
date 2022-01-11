#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE 511
#define MAX_SOCK 1024

char *EXIT_STRING = "exit";
char *START_STRING = "Connected to chat_server \n";
char *GAME_START = "\n가위(0) 바위(1) 보(2) 중에 선택하세요: ";

int num_chat = 0;
int clisock_list[MAX_SOCK];
int listen_sock;
int game_round = 0;
int new_game = 0;

char* stringToken(char* msg);
void addClient(int s, struct sockaddr_in *newcliaddr);
void removeClient(int s);
int set_nonblock(int sockfd);
int is_nonblock(int sockfd);
int result_rsp(int cli1, int cli2);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg) { perror(mesg); exit(1); }

int main(int argc, char *argv[]) {
   struct sockaddr_in cliaddr;
   char buf[MAXLINE];
   char *msg;
   char *str;
   char bufmsg[MAXLINE];
   int select[2];
   int i, j, nbyte, count;
   int cli1=0,cli2=0,result;
   int accp_sock, addrlen ,clilen;

   if(argc != 2) {
	printf("사용법: %s port \n", argv[0]);
	exit(0);
   }

   listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
   if(listen_sock == -1)
	errquit("tcp_listen fail");
   if(set_nonblock(listen_sock) == -1)
	errquit("ser_nonblock fail");

   for(count=0; ;count++) {
	if(count == 100000) {
	    putchar('.');
	    fflush(stdout); 
	    count = 0;
	}
	addrlen = sizeof(cliaddr);
	accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &clilen);
   	if(accp_sock == -1 && errno != EWOULDBLOCK)
	    errquit("accept fail");
	else if(accp_sock > 0) {
	   
	   if(is_nonblock(accp_sock) != 0 && set_nonblock(accp_sock) < 0)
		errquit("set_nonblock fail");
	   
	   addClient(accp_sock, &cliaddr);
	   send(accp_sock, START_STRING, strlen(START_STRING), 0); 
	   printf("%d 번째 사용자 추가 \n", num_chat);
       }
             
       	
        send(accp_sock, GAME_START, strlen(GAME_START), 0);
       
  	for(i=0; i < num_chat; i++) {
	  errno = 0;
	  nbyte = recv(clisock_list[i],buf, MAXLINE, 0);
	  if(nbyte == 0) {
		removeClient(i);
		continue;
	  } 
	  else if(nbyte == -1 && errno == EWOULDBLOCK)
		continue;
	  if(strstr(buf, EXIT_STRING) != NULL) {
	 	removeClient(i);
		continue;
	  }		

	  buf[nbyte] = 0;
	  printf("%s ", buf);
	  str = stringToken(buf);
	  select[i] = atoi(str);
	  
	  if(new_game == 1 && i == num_chat-1) {
	    	if(select[0]==1 && select[1]==1){
		    game_round=0;
		    new_game = 0;
		    for(j=0; j<num_chat; j++)
			send(clisock_list[j],GAME_START,strlen(GAME_START),0);
		        continue;
		}else {
		    msg = "Good bye. 게임종료.\n";
		    for(j=0; j<num_chat; j++){
			send(clisock_list[j],msg,strlen(msg),0);
			removeClient(j);
			continue;
		    }
		    puts("Good bye. 게임종료.\n");
		    exit(0);
		 }   
	 } 
	
	   if(i!=0 && new_game == 0) {
             result = result_rsp(select[0],select[1]);

	     if(result == 0){
	   	msg="비겼습니다!\n";
	   	for(i=0; i<num_chat; i++){ 
	   	    send(clisock_list[i],msg,strlen(msg),0);   
		}
	     }
	     else if(result == 1){
		sprintf(bufmsg,"이겼습니다! 상대방이 낸 것: %d\n", select[1]);
	  	send(clisock_list[0],bufmsg,strlen(bufmsg),0);
		sprintf(bufmsg,"졌습니다! 상대방이 낸 것: %d\n", select[0]);
	  	send(clisock_list[1],bufmsg,strlen(bufmsg),0);
	  	game_round++;	
		cli1++;
	     }	
	     else{
	  	sprintf(bufmsg,"졌습니다! 상대방이 낸 것: %d\n",select[1]);
	  	send(clisock_list[0],bufmsg,strlen(bufmsg),0);
	  	sprintf(bufmsg,"이겼습니다! 상대방이 낸 것: %d\n",select[0]);
	   	send(clisock_list[1],bufmsg,strlen(bufmsg),0);	
	   	game_round++;
		cli2++;
	     }   	 
	  }

	  if(game_round == 3 && new_game == 0 ) {
		msg = "==============================================\n";
		for(j=0; j<num_chat; j++) 
		    send(clisock_list[j], msg, strlen(msg), 0);

	        if(cli1 >= 2) {
		    msg="축하합니다! 게임에서 이겼습니다.\n";
		    send(clisock_list[0], msg, strlen(msg),0);
		    msg="아쉽네요! 게임에서 졌습니다.\n";
		    send(clisock_list[1], msg, strlen(msg),0);
		}
		if(cli2 >= 2) {
		    msg="아쉽네요! 게임에서 졌습니다.\n";
		    send(clisock_list[0], msg, strlen(msg),0);
		    msg="축하합니다! 게임에서 이겼습니다.\n";
		    send(clisock_list[1], msg, strlen(msg),0);
		}
	        msg = "새로운 게임을 원하면 1, 아니면 2를 선택하세요";
		for(j=0; j<num_chat; j++) 
		    send(clisock_list[j],msg,strlen(msg),0);
		new_game++;
	    }
    	}     
   } 
}

char *stringToken(char* msg) {
   char *str;
   str = strtok(msg," ");
   str = strtok(NULL, " ");
  
   return str;  
}

void addClient(int s, struct sockaddr_in *newcliaddr) {
  char buf[20];
  inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
  printf("new client: %s\n", buf);
  clisock_list[num_chat] = s;
  num_chat ++;
}

void removeClient(int s) {
  close(clisock_list[s]);
  if(s != num_chat-1)
      clisock_list[s] = clisock_list[num_chat-1];
  num_chat--;
  printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_chat);
}

int is_nonblock(int sockfd) {
  int val;

  val = fcntl(sockfd, F_GETFL, 0);
  if(val & O_NONBLOCK)
	return 0;
  return -1;
}

int set_nonblock(int sockfd) {
  int val;
  
  val = fcntl(sockfd, F_GETFL, 0);
  if(fcntl(sockfd, F_SETFL,val | O_NONBLOCK) == -1 ) 
	return -1;
  return 0;
}

int result_rsp(int cli1, int cli2) {
  if(cli1 == cli2) 
     return 0;
  else if(cli1 % 3 == (cli2+1)%3)
     return 1;
  else
     return -1;
}


int tcp_listen(int host, int port, int backlog) {
  int sd;
  struct sockaddr_in servaddr;

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd == -1) {
     perror("socket fail");
     exit(1);
  }

  bzero((char*)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(host);
  servaddr.sin_port = htons(port);
  if(bind(sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
     perror("bind fail");
     exit(1);
  }
  listen(sd, backlog);
  return sd;
}

