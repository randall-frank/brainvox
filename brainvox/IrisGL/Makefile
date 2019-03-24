#
#	Emulation layer for IrisGL over OpenGL
#
include ../config

FILES = igl_window.c igl_draw.c igl_events.c igl_pup.c igl_text.c \
	close.c filbuf.c flsbuf.c name.c open.c pix.c rdwr.c rle.c row.c

OBJECTS = igl_window.$(OBJ_SUFFIX) igl_draw.$(OBJ_SUFFIX) \
	igl_events.$(OBJ_SUFFIX) igl_text.$(OBJ_SUFFIX) myfont.$(OBJ_SUFFIX) \
	igl_pup.$(OBJ_SUFFIX)
SGIOBJS = close.$(OBJ_SUFFIX) filbuf.$(OBJ_SUFFIX) flsbuf.$(OBJ_SUFFIX) \
	name.$(OBJ_SUFFIX) open.$(OBJ_SUFFIX) pix.$(OBJ_SUFFIX) \
	rdwr.$(OBJ_SUFFIX) rle.$(OBJ_SUFFIX) row.$(OBJ_SUFFIX) \
	sgilib.$(OBJ_SUFFIX)

ifeq ($(OS),osx)
FONT = "-*-courier-medium-r-normal-*-14-*-*-*-*-*-*-14"
else
FONT = "-*-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"
endif

all: libsgiimg.$(LIB_SUFFIX) libIrisGL.$(LIB_SUFFIX)

ifneq ($(OS),WIN32)
capturexfont : capturexfont.c
	$(CC) $(CFLAGS) capturexfont.c -o capturexfont $(XLIB) $(CLIB)

myfont.c: capturexfont
	./capturexfont $(FONT) myfont myfont.c
endif

libsgiimg.$(LIB_SUFFIX) : $(SGIOBJS)
	$(AR) $(ARP)libsgiimg.$(LIB_SUFFIX) $(SGIOBJS)
	$(RANLIB) $@
	-mkdir ../ext_libs
	cp libsgiimg.$(LIB_SUFFIX) ../ext_libs

libIrisGL.$(LIB_SUFFIX) : $(OBJECTS)
	$(AR) $(ARP)libIrisGL.$(LIB_SUFFIX) $(OBJECTS)
	$(RANLIB) $@
	-mkdir ../ext_libs
	cp libIrisGL.$(LIB_SUFFIX) ../ext_libs

test: test.$(OBJ_SUFFIX) libIrisGL.$(LIB_SUFFIX)
	$(CC) $(CFLAGS) -o test$(EXE_SUFFIX) test.$(OBJ_SUFFIX) -L. $(GLIB) $(XLIB) $(CLIB)

clean:
	rm -f *.$(OBJ_SUFFIX) core libIrisGL.$(LIB_SUFFIX) \
		libsgiimg.$(LIB_SUFFIX) test capturexfont a.out

install: libIrisGL.$(LIB_SUFFIX) libsgiimg.$(LIB_SUFFIX)
	cp image.h ../ext_libs
	cp sgilib.h ../ext_libs
