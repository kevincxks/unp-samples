


## socket选项小记 


主要函数：
```c
int getsockopt(...)
int setsockopt(...)
```


* SO_ERROR

socket发生错误时会设置该值



* SO_KEEPALIVE

如果两小时内没有数据交换，就会给对端发送探测包

(1) 对端ack回应

(2) 对端RST：对端已崩溃，socket待处理错误：ECONNRESET

(3) 对端没响应:重传8个探测包，相隔75s，如果没响应ETIMEOUT，如果收到ICMP错误则返回响应错误（比如EHOSTUNREACH）




* SO_LINGER

 close默认情况下会立即返回，并发送缓冲区残留数据

(1) l_onoff为0，则该选项关闭，l_linger的值被忽略，close()用上述缺省方式关闭连接。

(2) l_onoff非0，l_linger为0，丢弃缓冲区数据，直接发送reset

(3) l_onoff非0，l_linger非0，close阻塞，直到数据发完且已被确认或者延滞时间到，返回EWOULDBLOCK




* SO_REUSEADDR

一般情况下，如果仍存在连接占用了某个端口，是不能bind的（常见的情况就是timewait）
该选项使得可以在这种情况下bind, 且可以bind同一个端口（对于TCP来说，ip地址需要不同)
一般使用上，可以bind多个特定的ip地址，再bind一个通配ip



* SO_RECVBUF和SO_SNDBUF

缓冲区大小



* SO_RECLOWAT和SO_SNDLOWAT

低水位标记，select会用到



* TCP_NODELAY

**Nagle算法**: 如果某个连接上存在待ack的数据，那么小分组（小于MSS）不会发送直到ack

**ACK延滞算法（delayed ACK algorithm)**： 收到数据不立即ack, 而是等一小段时间（50-200ms)，期待能携带数据

这个选项可以禁用Nagle







