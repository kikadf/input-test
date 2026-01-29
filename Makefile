CC = 		gcc
CFLAGS = 	-g -Wall -I/usr/include
LDFLAGS =	-L/lib -L/usr/lib
UNAME := 	$(shell uname)

ifeq ($(UNAME),FreeBSD)
	CC =		clang
	CFLAGS +=	-I/usr/local/include
	LDFLAGS +=	-L/usr/local/lib
endif

ifeq ($(UNAME),OpenBSD)
	CC =	clang
	CFLAGS +=	-I/usr/local/include
	LDFLAGS +=	-L/usr/local/lib
endif

ifeq ($(UNAME),NetBSD)
	CFLAGS +=	-I/usr/pkg/include
	LDFLAGS +=	-L/usr/pkg/lib -R/usr/pkg/lib
endif

itest:
	$(CC) $(CFLAGS) $(LDFLAGS) -o itest itest.c -ludev -linput

all: itest

clean:
	rm -f itest
