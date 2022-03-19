#include <stdio.h>
#include <sys/socket.h>



extern "C" {
#include <unp.h>
}

#define MAXCHAR 1024
void my_echo(int fd) {
  char buf[MAXCHAR];
  ssize_t n;
  while (true) {
    if ((n = read(fd, buf, MAXCHAR)) > 0) {
      writen(fd, buf, n);
    // 需要处理系统调用被中断的情况，此处重新发起即可
    } else if (n == -1 && errno == EINTR) {
      continue;
    } else if (n == -1) {
      err_sys("str_echo: read error");
      // 对于已经收到FIN的socket调用read会返回0,多次调用都会返回0
    } else {
      return;
    }
  }

}


void sig_chld(int signo) {
  pid_t pid;
  int stat;

  // pid = wait(&stat);
  // printf("child %d terminalted\n", pid);
  // fflush(stdout);

  // 一定要使用waitpid 因为信号不会排队
  // -1 表示的等待第一个终止的子进程
  // WNOHANG表示如果没有立刻返回
  while ((pid = waitpid(-1, &stat, WNOHANG)) != -1) {
    printf("child %d terminalted\n", pid);
    fflush(stdout);
  }
}

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


  // 注册SIGCHLD信号处理函数，防止子进程变为僵尸进程 
  Signal(SIGCHLD, sig_chld);


  while (true) {
    clilen = sizeof(cli_addr);
    // accept的实质是获取三次握手完成的连接
    // 存在一种可能就是在accept之前被客户端reset掉了，此时POSIX要求返回一个ECONNABORTED错误
    // 如果服务端直接崩溃了，即不可达，那么客户端发送消息将持续重传最终返回ETIMEDOUT
    // 如果服务端崩溃后重启，则收到客户端消息会直接回送reset
    if ((connfd = accept(listenfd, (SA *)&cli_addr, &clilen)) < 0) {
      // 需要处理被中断的情况，比如捕获到信号时
      if (errno == EINTR) {
        continue;
      } else {
        err_sys("accept error");
      }
    }
    pid_t pid;

    printf("connect from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    fflush(stdout);

    // 如果我们把子进程kill掉, 那么子进程会向客户端发送FIN进入半关闭状态，此时如果再收到消息会直接回reset
    // 半关闭状态会使得对方的read调用返回0（EOF）
    // 如果向已经收到reset的socket写东西，则内核会发一个SIGPIPE信号，默认终止程序
    // 如果捕获了信号或者忽略该信号，写操作会返回EPIPE错误
    // 第一次向FIN写会收到reset,第二次才会引发信号，前者是正常行为，后者则是错误
    if ((pid = fork()) == 0) {
      
      // 文件描述符会在父子进程拷贝，close会使得文件表项引用数减一，减到0才会发送FIN
      Close(listenfd);
      my_echo(connfd);

      // 子进程结束会给父进程一个SIGCHLD信号
      // 进程退出也会关闭所有的文件描述符
      exit(0);
    }
    
    // 此处如果没有close,则引用数只能减到1,导致客户端关闭后只能处在半关闭状态
    Close(connfd);
  }

  return 0;
}

