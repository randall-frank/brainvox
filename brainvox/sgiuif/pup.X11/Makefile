FILES =  README* Makefile pup.h pup.c

CFLAGS = -O 

libpup.a : pup.o
	ar ruvs libpup.a pup.o

pup.o : pup.c

pup.o : pup.h

testpup : libpup.a testpup.c
	$(CC) $(CFLAGS) -s -o testpup testpup.c libpup.a -lgl_s

testpupf : libpup.a testpupf.f
	f77 -O -o testpupf testpupf.f libpup.a -lfgl -lgl_s
	strip testpupf

tar: $(NROFF) $(FILES)
	tar cvf pup.tar $(FILES) $(NROFF)

.SUFFIXES: .1 .man .c .o

.man.1:
	nroff -man $*.man > $*.1

clean:
	rm -f make.out libpup.a pup.tar *.o *.1
