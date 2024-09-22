#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<iostream>
#define PORT "8080"
#define BACKLOG 10
using namespace std;
void sigchld_handler(int s){
  int saved_errno=errno;
  while(waitpid(-1,NULL,WNOHANG)>0);
  errno=saved_errno;
}
void * get_in_addr(sockaddr *sa){
  if(sa->sa_family==AF_INET){
    return &(((sockaddr_in*)sa)->sin_addr);
  }
  return &(((sockaddr_in6*)sa)->sin6_addr);
}
int main(void){
  int sockfd,new_fd;
  addrinfo hints,*servinfo,*p;
  sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes =1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_flags=AI_PASSIVE;
  if((rv=getaddrinfo(NULL,PORT,&hints,&servinfo))!=0){
    cout<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
    return 1;
  }
  for(p=servinfo;p!=NULL;p=p->ai_next){
    if((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
      perror("server: socket");
      continue;
    }
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int))==-1){
      perror("setsockopt");
      exit(1);
    }
    if(bind(sockfd,p->ai_addr,p->ai_addrlen)==-1){
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo);
  if(p==NULL){
    cout<<"server: failed to bind"<<endl;
    exit(1);
  }
  if(listen(sockfd,BACKLOG)==-1){
    perror("listen");
    exit(1);
  }
  sa.sa_handler=sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=SA_RESTART;
  if(sigaction(SIGCHLD,&sa,NULL)==-1){
    perror("sigaction");
    exit(1);
  }
  cout<<"server: waiting for connections..."<<endl;
  while(true){
    sin_size=sizeof their_addr;
    new_fd=accept(sockfd,(sockaddr*)&their_addr,&sin_size);
    if(new_fd==-1){
      perror("accept");
      continue;
    }
    inet_ntop(their_addr.ss_family,get_in_addr((sockaddr*)&their_addr),s,sizeof s);
    cout<<"server: got connection from "<<s<<endl;
    if(!fork()){
      close(sockfd);
          string res = "{\"hello\":\"himanshu\"}";
        string httpRes = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: application/json\r\n";
        string finalStr = httpRes + "Content-Length: " + to_string(res.size()) + "\r\n\r\n" + res;
        const char* cStyleString = finalStr.c_str();
      if(send(new_fd,cStyleString,strlen(cStyleString),0)==-1)
        perror("send");
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }
  return 0;
}