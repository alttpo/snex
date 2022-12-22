CFLAGS+=-std=c++14

ifdef COMSPEC
  EXESUFFIX?=.exe
  RM?=del /f
  MV?=move
else
  EXESUFFIX=
  RM?=rm -f
  MV?=mv -f
endif

all: server client

server$(EXESUFFIX): server.o xpipc.o
	$(CC) $(CFLAGS) $^ -o $@

client$(EXESUFFIX): client.o xpipc.o
	$(CC) $(CFLAGS) $^ -o $@

server.o: server.cpp xpipc.h xplat.h

xpipc.o: xpipc.cpp xpipc.h xplat.h

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) server$(EXESUFFIX)
	$(RM) client$(EXESUFFIX)
