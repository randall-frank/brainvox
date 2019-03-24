
include ../config

CFILES = flip.c light.c flipobj.c swirl.c \
	event.c trackball.c ui.c vect.c hash.c
HFILES = event.h trackball.h ui.h flip.h light.h vect.h hash.h
OBJECTS = flip.o light.o flipobj.o event.o trackball.o ui.o \
	vect.o hash.o

surfview : $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o surfview $(LCL_LIB) \
		-L../libs $(GLIB) $(XLIB) $(CLIB)

clean:
	-rm -f *.o core surfview

install:
	cp surfview ../bin
