CFLAGS = -pipe -O2 -Wall -ansi -pedantic
CC = gcc
TESTS = t1.test t2.test t3.test t4.test t5.test t6.test t7.test t8.test \
	t9.test t10.test

.SUFFIXES: .c .test
.c.o :
	$(CC) $(CFLAGS) -o $@ -c $<
.c.test :
	$(CC) -o $@ $(CFLAGS) $< as274c.o misc.o $L -lm

reg : $(TESTS)
	-./t1.test | diff t1.expect -
	-./t2.test | diff t2.expect -
	-./t3.test | diff t3.expect -
	-./t4.test | diff t4.expect -
	-./t5.test | diff t5.expect -
	-./t6.test | diff t6.expect -
	-./t7.test | diff t7.expect -
	-./t8.test | diff t8.expect -
	-./t9.test | diff t9.expect -
	- ./t10.test 2  2 1e-12 100 10 | diff t10a.expect -
	- ./t10.test 20 5 1e-12 100 10 | diff t10b.expect -


t1.test : as274c.o misc.o t1.c
t2.test : as274c.o misc.o t2.c
t3.test : as274c.o misc.o t3.c
t4.test : as274c.o misc.o t4.c
t5.test : as274c.o misc.o t5.c
t6.test : as274c.o misc.o t6.c
t7.test : as274c.o misc.o t7.c
t8.test : as274c.o misc.o t8.c
t9.test : as274c.o misc.o t9.c
t10.test : as274c.o misc.o t10.c

as274c.o : as274c.c as274c.h
misc.o : misc.c misc.h

clean : 
	rm -f *.o a.out $(TESTS)

