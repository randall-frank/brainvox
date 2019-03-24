#
# Makefile for testing
#
# set your compiler and its flags
# there are several defines that you can use for test-all.c
#
# defines for fftn.c
# -DFFT_NOFLOAT or -DFFT_NODOUBLE to avoid compiling that precision
# -DFFT_RADIX4 if you only want radix 2, radix 4 transforms
# -DSUN_BROKEN_REALLOC if your realloc() function doesn't accept NULL
#

# gcc
CC = cc
CFLAGS = -ansi -O2 

# AIX
# CC = xlc
# CFLAGS = -O2

# ------
.SUFFIXES:
.SUFFIXES: .o .c .h

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDE) $<

# ------

test-fftn:	fftn.o test-fftn.o
	$(CC) -o $@ fftn.o test-fftn.o -lm

test-all:	fftn.o test-all.o
	$(CC) -o $@ fftn.o test-all.o -lm

test-all.o:	test-all.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDE) $(TEST_DEFS) $<

fftn.o:  fftn.c fftn.h

clean:
	rm -f *.o *~

veryclean:
	rm -f *.o *~ test-fftn test-all
