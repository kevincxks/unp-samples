

#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
extern "C" {
#include <unp.h>
}




int main (int argc, char *argv[]) {



  int clientfds[5];

  sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  serv_addr.sin_port = htons(8888);
  for (int i = 0; i < 5; i++) {

    clientfds[i] = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(clientfds[i], (SA *)&serv_addr, sizeof(serv_addr));
  
  }
  str_cli(stdin, clientfds[0]);
  
  return 0;
}
