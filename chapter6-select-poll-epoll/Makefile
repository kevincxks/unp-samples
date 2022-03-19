

CPP=g++
OPTS=-std=c++11
CPPFLAGS=-Wall -g $(OPTS)

LINK=-lunp

BINARY=echo_client echo_server_select echo_server_poll echo_server_epoll


all: $(BINARY)
echo_client: echo_client.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

echo_server_select: echo_server_select.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

echo_server_poll: echo_server_poll.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

echo_server_epoll: echo_server_epoll.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

%.o: %.cc
	$(CPP) $(CPPFLAGS) -c -o $@ $^

clean:
	rm -rf $(BINARY) *.o
