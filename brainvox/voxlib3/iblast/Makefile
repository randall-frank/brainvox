#
#	Makefile for the iblast demo program
#

LIBS = -L.. -lIRIX5 -lgl_s -lm 
CFLAGS = -O -I..
OBJECTS = main.o loadsave.o svimg.o cmdline.o palettes.o

iblast : $(OBJECTS) ../libIRIX5.a
	cc $(CFLAGS) $(OBJECTS) -o iblast  $(LIBS)

$(OBJECTS) : iblast.h ../voxel.h

