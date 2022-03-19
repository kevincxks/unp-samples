
#include <bits/types/FILE.h>
#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string>
extern "C" {
#include <unp.h>
}


using namespace std;



void my_str_cli(FILE *fp, int sockfd) {

  fd_set rset;
  int maxfdp1;

  char buf[MAXLINE];

  bool stdineof = false;

  FD_ZERO(&rset);
  while (true) {

    if (!stdineof) {
      FD_SET(fileno(fp), &rset);
    }

    FD_SET(sockfd, &rset);
    maxfdp1 = max(sockfd, fileno(fp)) + 1;
    int n = Select(maxfdp1, &rset, nullptr, nullptr, nullptr);

    if (FD_ISSET(sockfd, &rset)) {
      if ((n = Read(sockfd, buf, MAXLINE)) == 0) {
        if (stdineof) {
          return;
        } else {
          err_quit("str_cli: server terminated prematurely");
        }
      }
      Write(fileno(stdout), buf, n);
    }

    if (FD_ISSET(fileno(fp), &rset)) {
      if ((n = Read(fileno(fp), buf, MAXLINE)) == 0) {
        stdineof = true;
        // shutdown可以直接关闭不需要引用计数为0
        shutdown(sockfd, SHUT_WR);
        FD_CLR(fileno(fp), &rset);
        continue;
      }
      Write(sockfd, buf, n);
    }

  }
} 


int main (int argc, char *argv[]) {
  int clientfd;

  clientfd = Socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  serv_addr.sin_port = htons(8888);
  Connect(clientfd, (SA *)&serv_addr, sizeof(serv_addr));

  my_str_cli(stdin, clientfd);
  
  return 0;
}
