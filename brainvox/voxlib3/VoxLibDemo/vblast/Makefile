#
#	Makefile for GL based voxlib demo program
#

LIBS = -L../voxlib -lvox -lgl_s -lm 
CFLAGS = -O  -I../voxlib
OBJECTS = main.o render.o loadsave.o callbacks.o

vblast : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o vblast  $(LIBS)

