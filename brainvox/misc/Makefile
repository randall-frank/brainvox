
PROGS = Calc_Surface confide3d kappatest match_points match_kappa
LIBS = -lm
EXTRAS = fileio.o name_changer.o roi_utils.o polyfill_c.o

CFLAGS = -O

all : $(PROGS)

Calc_Surface : Calc_Surface.c
	$(CC) $(CFLAGS) Calc_Surface.c -o Calc_Surface $(LIBS)
	cp Calc_Surface Calc_Surface_fast

confide3d : confide3d.c
	$(CC) $(CFLAGS) confide3d.c -o confide3d $(LIBS)

kappatest : kappatest.c
	$(CC) $(CFLAGS) kappatest.c -o kappatest $(LIBS)
	
match_points : match_points.c $(EXTRAS)
	$(CC) $(CFLAGS) match_points.c -o match_points $(EXTRAS) $(LIBS)

match_kappa : match_kappa.c $(EXTRAS)
	$(CC) $(CFLAGS) match_kappa.c -o match_kappa $(EXTRAS) $(LIBS)

clean :
	rm -f *.o code $(PROGS) Calc_Surface_fast *.exe

tar :
	rm -f misc.tar misc.tar.gz
	tar cvf misc.tar *.c *.h Makefile
	gzip misc.tar

install :
	echo "Misc does not support install"
