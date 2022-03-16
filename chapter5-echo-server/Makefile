

CPP=g++
OPTS=-std=c++11
CPPFLAGS=-Wall -g $(OPTS)

LINK=-lunp

BINARY=echo_client echo_server echo_multi_client


all: $(BINARY)
echo_client: echo_client.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

echo_server: echo_server.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

echo_multi_client: echo_multi_client.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LINK)

%.o: %.cc
	$(CPP) $(CPPFLAGS) -c -o $@ $^

clean:
	rm -rf $(BINARY) *.o
