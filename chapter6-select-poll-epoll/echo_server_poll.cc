#include <arpa/inet.h>
#include <cstdio>
#include <limits.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
extern "C" {
#include <unp.h>
}

int main(int argc, char *argv[]) {

  int listenfd;
  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(8888);

  Bind(listenfd, (SA *)&serv_addr, sizeof(serv_addr));

  Listen(listenfd, 5);

  // poll使用一个数组
  pollfd clients[_SC_OPEN_MAX];

  clients[0].fd = listenfd;
  // event为关注的事件
  // POLLRDNORM表示普通数据可读，所有TCP正规数据都是普通数据
  clients[0].events = POLLRDNORM;

  for (int i = 1; i < _SC_OPEN_MAX; i++) {
    clients[i].fd = -1;
  }
  int maxi = 0;

  int nready;
  sockaddr_in client_addr;
  socklen_t client_len;
  int connfd;
  int i;
  ssize_t n;
  char buf[MAXLINE];
  while (true) {
    // 返回就绪的描述符个数，超时则为0
    // 需要用户传入描述符的个数，因此没有select的描述符数量限制
    nready = poll(clients, maxi + 1, INFTIM);

    // revents是发生的事件
    if (clients[0].revents & POLLRDNORM) {
      client_len = sizeof(client_addr);
      connfd = Accept(listenfd, (SA *)&client_addr, &client_len);
      printf("connect from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      fflush(stdout);

      for (i = 1; i < _SC_OPEN_MAX; i++) {
        if (clients[i].fd < 0) {
          clients[i].fd = connfd;
          break;
        }
      }
      if (i == _SC_OPEN_MAX) {
        err_quit("too many clients");
      }

      clients[i].events = POLLRDNORM;
      if (maxi < i)
        maxi = i;

      if (--nready <= 0)
        continue;
    }

    for (i = 1; i <= maxi; i++) {
      if (clients[i].fd < 0)
        continue;
      // TCP连接存在错误可以是普通数据也可以是错误（POLLERR）
      if (clients[i].revents & (POLLRDNORM | POLLERR)) {
        if ((n = read(clients[i].fd, buf, MAXLINE)) < 0) {
          if (errno == ECONNRESET) {
            Close(clients[i].fd);
            clients[i].fd = -1;
          } else {
            err_sys("read error");
          }
        } else if (n == 0) {
          printf("client closed\n");
          fflush(stdout);
          Close(clients[i].fd);
          clients[i].fd = -1;
        } else {
          Writen(clients[i].fd, buf, n);
        }
        if (--nready <= 0)
          break;
      }
    }
  }

  return 0;
}
