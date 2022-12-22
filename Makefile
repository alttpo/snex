.cpp.o:
	$(CC) $(CFLAGS) -c $<

all: client server

server: server.o
	$(CC) server.o -o $@

client: client.o
	$(CC) client.o -o $@
