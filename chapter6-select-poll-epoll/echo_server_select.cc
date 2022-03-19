#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>

extern "C" {
#include <unp.h>
}

#define MAXCHAR 1024

int main (int argc, char *argv[]) {

  int listenfd, connfd;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);


  sockaddr_in serv_addr, cli_addr;

  socklen_t clilen;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(8888);


  Bind(listenfd, (SA *)&serv_addr, sizeof(serv_addr));

  // 5表示backlog参数，该值往往与两个队列（已完成连接队列和未完成连接队列）关联
  Listen(listenfd, 5);


  int clients[FD_SETSIZE];

  // 最大描述符限制
  for (int i = 0; i < FD_SETSIZE; i++) clients[i] = -1;

  fd_set allset, rset;

  FD_ZERO(&allset);

  // 先把listenfd加上
  FD_SET(listenfd, &allset);

  int nready;
  int maxfd = listenfd;
  int maxi = -1;
  ssize_t n;
  char buf[MAXLINE];
  while (true) {

    // select需要手动重置感兴趣的fd
    rset = allset;

    // 满足下列四个条件之一，socket可读
    // （1）接收缓冲区数据大于低水位标记
    // （2）读半部关闭，即收到FIN
    // （3）为监听socket且已完成连接不为0
    // （4）有错误待处理，read返回-1
    //
    // 满足下列四个条件之一，socket可写
    // （1）发送缓冲区空间大于低水位标记
    // （2）写半部关闭
    // （3）非阻塞connect已连接，或者失败
    // （4）有错误待处理，write返回-1
    //
    // 异常条件：
    // （1）存在带外数据或者仍处于带外标记

    // 传入最大的描述符+1， 以及readset, writeset, exceptset
    // 返回就绪的描述符个数
    nready = Select(maxfd + 1, &rset, nullptr, nullptr, nullptr);

    if (FD_ISSET(listenfd, &rset)) {
      clilen = sizeof(cli_addr);
      connfd = Accept(listenfd, (SA *)&cli_addr, &clilen);

      printf("connect from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
      int i = 0;
      for (i = 0; i < FD_SETSIZE; i++) {
        if (clients[i] == -1) {
          clients[i] = connfd;
          break;
        }
      }
      if (i == FD_SETSIZE) {
        err_quit("too many clients");
      }

      FD_SET(connfd, &allset);

      if (connfd > maxfd) maxfd = connfd;
      if (i > maxi) maxi = i;

      if (--nready <= 0) continue;
    }

    // 还是需要遍历
    for (int i = 0; i <= maxi; i++) {
      if (clients[i] < 0) continue;
      if (FD_ISSET(clients[i], &rset)) {
        // select和poll其实是LT模式的，即如果一次没读完缓冲区的，下次还是会就绪
        if ((n = Read(clients[i], buf, MAXLINE)) == 0) {
          close(clients[i]);
          // 记得清除掉fd
          FD_CLR(clients[i], &allset);
          clients[i] = -1;
        } else {
          Writen(clients[i], buf, n);
        }
        if (--nready <= 0) break;
      }
    }
  }
  return 0;
}

