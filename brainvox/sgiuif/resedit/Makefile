#    ======================================================================
#       Copyright (C) 1991 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for resedit
#
OPT = -O
.f.o:
	f77 $(OPT) -c $*.f

PROG = resedit

FILES = $(PROG)uif.r $(PROG).f

OBJECTS = $(PROG)uif.o $(PROG).o rjf.o

F77FLAGS =

$(PROG) : $(OBJECTS) ../libuif.a
	f77 -O $(OBJECTS) -o $(PROG) \
		-L.. -luif \
		-lfgl -lgl_s -lfm_s -lgutil

$(OBJECTS) : sgiuif.h

sgiuif.h :
	cp ../sgiuif.h .

$(PROG)uif.o : $(PROG)uif.r
	rmaker $*.r $*.f $*.inc
	f77 $(OPT) -c $*.f

cleanup:
	-rm *.o
	-du
install:
	strip $(PROG)
	cp $(PROG) ~/bin
