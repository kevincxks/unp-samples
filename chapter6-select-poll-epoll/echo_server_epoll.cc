#include <arpa/inet.h>
#include <cstdio>
#include <limits.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
extern "C" {
#include <unp.h>
}
#define MAX_EVENT_NUMBER 1024

int setnonblocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void addfd(int epollfd, int fd, bool enable_et) {
  epoll_event event;
  // 用户数据, epoll_wait返回时会携带该数据
  event.data.fd = fd;
  // 关注的事件
  event.events = EPOLLIN;
  // 如果是et模式需要带上EPOLLET flag
  if (enable_et) {
    event.events |= EPOLLET;
  }
  // 这里暂时不是很明白为什么不怕析构, 猜测应该是系统调用会拷贝用户空间的原因
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  // et模式必须要配合nonblocking,否则有可能永久阻塞
  if (enable_et)
    setnonblocking(fd);
}

// level trigger模式，如果没读完会反复触发就绪事件
void lt(epoll_event *, int, int, int);

// edge trigger, 新数据来了之后只会触发一次就绪，需要读完
void et(epoll_event *, int, int, int);

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

  int epollfd;

  // 这个参数现在ignored了，但是要大于0
  if ((epollfd = epoll_create(5)) < 0)
    err_sys("create epoll fail");

  epoll_event events[MAX_EVENT_NUMBER];

  addfd(epollfd, listenfd, true);

  while (true) {


    // events只负责输出检测到的就绪事件
    // 返回就绪的文件描述符个数
    int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if (ret < 0)
      err_sys("epoll wait fail");

    lt(events, ret, epollfd, listenfd);
    // et(events, ret, epollfd, listenfd);
  }
  return 0;
}

void lt(epoll_event *events, int number, int epollfd, int listenfd) {

  int connfd;
  sockaddr_in cliaddr;
  socklen_t clilen;
  ssize_t n;
  char buf[MAXLINE];
  // lt模式下就是个增强版的poll
  for (int i = 0; i < number; i++) {
    int sockfd = events[i].data.fd;

    if (sockfd == listenfd) {
      clilen = sizeof(cliaddr);
      connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
      printf("connect from %s:%d\n", inet_ntoa(cliaddr.sin_addr),
             ntohs(cliaddr.sin_port));

      // lt模式
      addfd(epollfd, connfd, false);
    } else if (events[i].events & (EPOLLIN | EPOLLERR)) {
      printf("触发事件\n");
      if ((n = read(sockfd, buf, MAXLINE)) == 0) {
        // close会自动反注册事件，前提是引用数减为0
        close(connfd);
        printf("client closed\n");
      } else if (n < 0) {
        if (errno == ECONNRESET) {
          close(connfd);
          printf("client closed\n");
        } else {
          err_sys("read error");
        }
      } else {
        Writen(connfd, buf, n);
      }
    } else {
      printf("something else happen\n");
    }
  }
}

void et(epoll_event *events, int number, int epollfd, int listenfd) {

  int connfd;
  sockaddr_in cliaddr;
  socklen_t clilen;
  ssize_t n;
  char buf[MAXLINE];
  for (int i = 0; i < number; i++) {
    int sockfd = events[i].data.fd;

    if (sockfd == listenfd) {
      clilen = sizeof(cliaddr);
      connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
      printf("connect from %s:%d\n", inet_ntoa(cliaddr.sin_addr),
             ntohs(cliaddr.sin_port));

      addfd(epollfd, connfd, true);
    } else if (events[i].events & (EPOLLIN | EPOLLERR)) {

      printf("触发事件\n");
      // et模式下需要while读完数据
      while (true) {
        if ((n = read(sockfd, buf, MAXLINE)) == 0) {
          close(connfd);
          printf("client closed\n");
        } else if (n < 0) {
          if (errno == ECONNRESET) {
            close(connfd);
            printf("client closed\n");
          // 返回EAGIN或者EWOULDBLOCK就表明no data available right now，读完了
          } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            printf("read later\n");
            break;
          } else {
            err_sys("read error");
          }
        } else {
          Writen(connfd, buf, n);
        }
      }
    } else {
      printf("something else happen\n");
    }
  }
}
