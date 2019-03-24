#
#	This subdir contains routines used as utilities by the brainvox
#	set of programs.  make creates a library named: libvtrace.a
#
#    ======================================================================
#       Copyright (C) 1990 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#
include ../config

FILES =	shared.c sharedio.f imagestuff.f name_changer.f polyfill.f\
	svimg.c filenames.f set_cindex.c cfileio.c io_help.c bv_error.f\
	polyfill_c.c mesg_box.c tal_interface.c set_gamma.c fileio.f\
	keyboard.f match.c z_compress.c prefs.c nifti1.c

OBJECTS = shared.$(OBJ_SUFFIX) sharedio.$(OBJ_SUFFIX) imagestuff.$(OBJ_SUFFIX) name_changer.$(OBJ_SUFFIX) polyfill.$(OBJ_SUFFIX)\
	svimg.$(OBJ_SUFFIX) filenames.$(OBJ_SUFFIX) set_cindex.$(OBJ_SUFFIX) cfileio.$(OBJ_SUFFIX) bv_error.$(OBJ_SUFFIX)\
	polyfill_c.$(OBJ_SUFFIX) mesg_box.$(OBJ_SUFFIX) tal_interface.$(OBJ_SUFFIX) set_gamma.$(OBJ_SUFFIX) fileio.$(OBJ_SUFFIX)\
	keyboard.$(OBJ_SUFFIX) match.$(OBJ_SUFFIX) z_compress.$(OBJ_SUFFIX) prefs.$(OBJ_SUFFIX) io_help.$(OBJ_SUFFIX) nifti1.$(OBJ_SUFFIX)

ZFILES = adler32.c compress.c crc32.c gzio.c uncompr.c deflate.c \
	trees.c zutil.c inflate.c infback.c inftrees.c inffast.c 

ZHFILES = crc32.h deflate.h inffast.h inflate.h inftrees.h trees.h zutil.h \
	zconf.h zlib.h

ZOBJECTS = adler32.$(OBJ_SUFFIX) compress.$(OBJ_SUFFIX) crc32.$(OBJ_SUFFIX) gzio.$(OBJ_SUFFIX) uncompr.$(OBJ_SUFFIX) deflate.$(OBJ_SUFFIX) \
	trees.$(OBJ_SUFFIX) zutil.$(OBJ_SUFFIX) inflate.$(OBJ_SUFFIX) infback.$(OBJ_SUFFIX) inftrees.$(OBJ_SUFFIX) inffast.$(OBJ_SUFFIX) 

all: $(ZFILES)
	make libvtrace.$(LIB_SUFFIX)

$(ZFILES) :
	cp zlib_123/* .

libvtrace.$(LIB_SUFFIX) : $(OBJECTS) $(ZOBJECTS)
	$(AR) $(ARP)$@ $(OBJECTS) $(ZOBJECTS)
	$(RANLIB) $@

$(OBJECTS) : libvtrace.h vt_inc.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core libvtrace.$(LIB_SUFFIX) $(ZFILES) $(ZHFILES)

install:
	$(RANLIB) libvtrace.$(LIB_SUFFIX)
	cp libvtrace.$(LIB_SUFFIX) ../ext_libs
	cp vt_inc.f ../ext_libs
	cp prefs.f ../ext_libs
	cp keyboard_inc.f ../ext_libs
	cp keyboard_int.f ../ext_libs
	cp match.h ../ext_libs
	cp z_compress.h ../ext_libs
	cp zlib.h ../ext_libs
	cp zconf.h ../ext_libs
	cp nifti1.h ../ext_libs
