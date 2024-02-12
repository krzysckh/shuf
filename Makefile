OFILES = shuf.o
TARGET = shuf
PREFIX = /usr/local

UNAME != uname | tr '[A-Z]' '[a-z]'

CFLAGS = -Wall -Wextra -D_DEFAULT_SOURCE -D_BSD_SOURCE -std=c89 -pedantic

.if $(UNAME) == "linux"
LDFLAGS = -lbsd
.endif

.c.o:
	$(CC) $(CFLAGS) -c $<
all: manpage $(OFILES)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OFILES)
manpage:
	pod2man -s 1 -c $(TARGET) -n $(TARGET) < shuf.pod > shuf.1
clean:
	rm -f *.o $(TARGET) shuf.1 *.core
install:
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/bin
	cp shuf.1 $(PREFIX)/man/man1/

