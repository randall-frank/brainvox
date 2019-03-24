#
#	Makefile for the iblast demo program
#

LIBS = -L../voxlib -lvox -lgl_s -lm 
CFLAGS = -O -I../voxlib
OBJECTS = main.o loadsave.o svimg.o cmdline.o palettes.o

iblast : $(OBJECTS)
	cc $(CFLAGS) $(OBJECTS) -o iblast  $(LIBS)

$(OBJECTS) : iblast.h

