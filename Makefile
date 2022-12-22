CFLAGS+=-std=c++14

.cpp.o:
	$(CC) $(CFLAGS) -c $<

all: server client

server: server.o
	$(CC) server.o -o $@

client: client.o
	$(CC) client.o -o $@
