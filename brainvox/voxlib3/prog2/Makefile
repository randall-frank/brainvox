LIBS = -L.. -lvox -lgl_s -lm 
CFLAGS = -O  
OBJECTS = main.o render.o resample.o svbuf.o draw_rects.o

vblast : $(OBJECTS) ../libvox.a
	cc $(CFLAGS) $(OBJECTS) -o vblast  $(LIBS)
	strip vblast

$(OBJECTS) : ../voxel.h

