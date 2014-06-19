# This is modified work by Miroslav Madzarevic githubcode@osadmin.com as of May 1st 2002
# based originally on proxy-2.2.4 program by sparlin@openpro.org
 
# For debugging...
#CFLAGS=-D_REENTRANT -Wall -ggdb -DDEBUG

CFLAGS=-D_REENTRANT -Wall -O6
LIBS=-lpthread

# LIBS for a non posix OS, like Solaris...
#LIBS=-lpthread -lsocket -lresolv -lnsl -L./unpv12e -lunp
   
all: proxylb

proxylb:  tcp_listen.o tcp_connect.o sock_ntop.o list.o config.o util.o proxylb.o proxylb.h
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $@.o tcp_listen.o tcp_connect.o sock_ntop.o list.o config.o util.o $(LIBS)
	strip --strip-all $@   
clean: 
	rm -f *~ *.o proxylb
