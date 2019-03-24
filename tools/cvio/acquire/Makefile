#
#	$Id: Makefile 260 2001-05-18 22:23:14Z rjfrank $
#
CFLAGS = -g -I.. -I../zlib-1.1.3

LIBS = -L.. -L../zlib-1.1.3 -lcvio -lz -lm -laudio

SHELL = /bin/sh

#
#	Only under Irix with Digital Media libraries installed
#
all :
	@echo "OS type detected: "`uname`
	@if [ `uname` = "IRIX" ] && [ -d "/usr/include/dmedia" ]; \
		then make acquire_sgi ; \
	 elif [ `uname` = "IRIX64" ] && [ -d "/usr/include/dmedia" ]; \
		then make acquire_sgi ; \
	 else make acquire; \
	 fi

acquire : acquire.c
	$(CC) $(CFLAGS) -o acquire acquire.c -lm

acquire_sgi : acquire.c
	$(CC) $(CFLAGS) -Dirix -o acquire acquire.c $(LIBS)

install :
	cp acquire ../../bin
	cp *.pro ../../bin

clean :
	rm -f acquire core *.o 
