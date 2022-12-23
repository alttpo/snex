CFLAGS+=-std=c++14 -D_XOPEN_SOURCE

ifdef COMSPEC
  EXESUFFIX?=.exe
  RM?=del /f
  MV?=move
else
  EXESUFFIX=
  RM?=rm -f
  MV?=mv -f
  LDFLAGS+=-lrt -pthread
endif

all: server client

server$(EXESUFFIX): server.o xpipc.o
	$(CC) $^ -o $@ $(LDFLAGS)

client$(EXESUFFIX): client.o xpipc.o
	$(CC) $^ -o $@ $(LDFLAGS)

server.o: server.cpp xpipc.h xplat.h snex.h

client.o: client.cpp xpipc.h xplat.h snex.h

xpipc.o: xpipc.cpp xpipc.h xplat.h

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) server$(EXESUFFIX)
	$(RM) client$(EXESUFFIX)
