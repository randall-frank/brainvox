
ship :
	tar cf - C F README documentation Makefile \
		| compress | uuencode as274.tar.Z > as274.uue

backup :
	tar cf backup.`datestring`.tar C F
	compress *.tar

clean :
	cd C; make clean
	cd F; make clean

reg :
	cd C; make reg
	cd F; make reg

