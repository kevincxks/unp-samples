
#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
extern "C" {
#include <unp.h>
}


using namespace std;


void sig_pipe(int signo) {
  cout << "I've get a signal:" << signo << endl;
}



int main (int argc, char *argv[]) {


  int clientfd;

  clientfd = Socket(AF_INET, SOCK_STREAM, 0);


  sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  serv_addr.sin_port = htons(8888);
  Signal(SIGPIPE, sig_pipe);
  Connect(clientfd, (SA *)&serv_addr, sizeof(serv_addr));

  std::string tmp;
  std::cin >> tmp;
  // str_cli(stdin, clientfd);

  char buf[] = "hello";
  cout << writen(clientfd, buf, sizeof(buf)) << endl;
  cout << writen(clientfd, buf, sizeof(buf)) << endl;
  err_sys("error:");
  
  return 0;
}
