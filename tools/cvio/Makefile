#
# Unix makefile for IDL_CVIO DLM.
# $Id: Makefile 1455 2005-11-26 19:27:22Z rjfrank $ 
#
#

# 	The following macro might need to be edited if the location
#	of IDL is not the standard location on your system.
#IDL_DIR = $(IDL_DIR)


# The following macros establish general defaults. They are overridden
# in the "all" rule as needed to satisfy a given platform's requirements.

SHELL=/bin/sh
#DEBUG=-g
DEBUG=-O2

X_LD_FLAGS=
X_CFLAGS=

C_FLAGS=-I$(IDL_DIR)/external -I. $(DEBUG)
LIBS=-L. -lcvio -lz -lpthread -lm -ldl
LD=ld
OBJ_SUFFIX=o
EXE_SUFFIX=
LIB_SUFFIX=a
SO_SUFFIX=so
AR=ar -r
ARP=

ifeq ($(OS),WIN32)
C_FLAGS=-I$(IDL_DIR)/external -Gs -nologo -DWIN32 -I.
LIBS=-L. -lcvio -lz -lpthread
OBJ_SUFFIX=obj
EXE_SUFFIX=.exe
LIB_SUFFIX=lib
SO_SUFFIX=dll
AR=lib /nologo
ARP=/OUT:
endif

LIBCVIO		= libcvio.$(LIB_SUFFIX)
APPS		= cvio_cp$(EXE_SUFFIX) cvio_info$(EXE_SUFFIX) \
			cvio_ls$(EXE_SUFFIX) cvio_mk$(EXE_SUFFIX) \
			cvio_rm$(EXE_SUFFIX) cvio_cat$(EXE_SUFFIX) \
			cvio_attr$(EXE_SUFFIX) cvio_sample$(EXE_SUFFIX) \
			cvio_image_tag$(EXE_SUFFIX) cvio_build$(EXE_SUFFIX) \
			cvio_impulse$(EXE_SUFFIX) \
			cvio_volumize$(EXE_SUFFIX) cvio_math$(EXE_SUFFIX) \
			cvio_rtip_pretimed_image_tag$(EXE_SUFFIX) \
			cvio_regress$(EXE_SUFFIX) cvio_align$(EXE_SUFFIX)
MANFILES	= cvio_intro.1 \
		  cvio_cp.1 cvio_info.1 cvio_ls.1 cvio_mk.1 cvio_rm.1 \
		  cvio_cat.1 cvio_attr.1 cvio_sample.1 cvio_image_tag.1 \
	          cvio_build.1 cvio_impulse.1 \
	          cvio_lib.3 \
	          cvio_init.3 cvio_cleanup.3 \
                  cvio_get_error_message.3 \
		  cvio_listshmem.3 \
	          cvio_create.3 cvio_delete.3 \
		  cvio_open.3 cvio_close.3 \
		  cvio_read_samples.3 cvio_read_next_samples.3 \
		  cvio_add_samples.3 \
		  cvio_getattribute.3 cvio_setattribute.3 cvio_getattriblist.3 \
		  cvio_current_time.3 \
		  cvio_datatype.3 \
		  cvio_tell.3 cvio_seek.3 cvio_has_writer.3 cvio_max_length.3 
#
#	derived man page files...
#
MANCLEAN	= cvio_cleanup.3 cvio_delete.3 cvio_close.3 \
		  cvio_read_next_samples.3 cvio_add_samples.3 \
	          cvio_setattribute.3 cvio_getattriblist.3 cvio_seek.3 \
	          cvio_has_writer.3 cvio_max_length.3

ACQUIRE		= acquire/acquire

FILES = idl_cvio.c idl_cvio.dlm idl_cvio.export cvio_cat.c \
	idl_cvio.def Makefile cvio_lib.h cvio_lib.c readme.txt \
	cvio_cp.c cvio_info.c cvio_ls.c cvio_mk.c cvio_rm.c cvio_sample.c \
        test_cvio.pro cvio_attr.c test2_cvio.pro cvio_image_tag.c \
	cvio_rtip_pretimed_image_tag.c 75_base.c 75_base.f as274.h \
	274.c 274.f f2c.h \
	acquire cvio_build.c cvio_impulse.c idl_cvio.dsw idl_cvio.dsp \
	$(MANFILES) cvio_align.c air_align.c air_interface.c air_interface.h \
	cvio_volumize.c cvio_priv.h cvio_sock.c cvio_math.c macro.c \
	macro2c.c macroprotos.h macro2protos.h cvio_dynamic.h cvio_dynamic.c \
	cvio_regress.h cvio_regress.c regress_parse.c

.SUFFIXES: .obj .c

.c.obj :
	$(CC) /c $(C_FLAGS) $(X_CFLAGS) /Fo$*.obj $*.c

.c.o :
	$(CC) -c $(C_FLAGS) $(X_CFLAGS) $*.c

# The following is the default entry point. This section will determine 
# what system we are on and call make again with the correct flags set.

all :
	@echo "OS type detected: "`uname`
	@case `uname` in \
		"Linux" ) make allapps \
			"X_CFLAGS=-Wall" \
			;;\
		"Darwin" ) make allapps \
			;;\
		"CYGWIN_NT-5.1" ) make allapps \
			"OS=WIN32" \
			;;\
		* ) make allapps \
			;;\
	esac

allapps : $(LIBCVIO) $(APPS)

idl : $(LIBCVIO) $(APPS) $(ACQUIRE)
	@echo "OS type detected: "`uname`
	@if [ $${#IDL_DIR} = 0 ]; then \
		echo "IDL_DIR variable must be set for DLMs" ; \
	else \
	  case `uname` in \
	   "SunOS") if [ -d /proc ]; then \
	           	make idl_cvio \
			"X_CFLAGS=-K pic" \
			"X_LD_FLAGS=-B symbolic -z nodefs -G" ; \
		    else \
	      	      make idl_cvio "CC=acc" \
			"X_CFLAGS= -pic" \
			"X_LD_FLAGS= -assert pure-text" ; \
	   	    fi  \
		;; \
	   "AIX") make idl_cvio \
			"LD=cc" \
			"SO_EXT=a" \
			"X_CFLAGS=-qro -qroconst" \
			"X_LD_FLAGS= -bM:SRE -bnoentry -btextro -bE:idl_cvio.export -bI:$(IDL_DIR)/external/idl.export" ;;\
	   "HP-UX") make idl_cvio  \
			"SO_EXT= sl" \
			"X_CFLAGS=+z -Aa -D_HPUX_SOURCE" \
			"X_LD_FLAGS= -b -Brestricted +s" ;; \
	   "IRIX" )  make idl_cvio \
		       "X_LD_FLAGS=-Bsymbolic -shared -all -rdata_shared" ;;\
	   "IRIX64" )  make idl_cvio \
		       "X_LD_FLAGS=-Bsymbolic -shared -all -rdata_shared" ;;\
	   "OSF1" )  make idl_cvio \
			"X_CFLAGS=-float" \
			"X_LD_FLAGS=-expect_unresolved '*' -shared -all" ;;\
	   "Linux" ) make idl_cvio \
			"X_CFLAGS= -fPIC" \
			"X_LD_FLAGS= -shared -Bsymbolic --warn-once -noinhibit-exec " ;; \
	   "Darwin" ) make idl_cvio \
	   		"X_CFLAGS= -no-cpp-precomp -dynamic -fPIC -fno-common -D_REENTRANT" \
			"CC=gcc" \
			"LD=gcc" \
			"X_LD_FLAGS= -bundle -flat_namespace -undefined suppress" ;; \
	   *) echo "This system is not supported" ;; \
	  esac \
	fi
#
# The following is the entry point for the recursive build
#
idl_cvio : idl_cvio.$(SO_SUFFIX)
	@date

idl_cvio.$(SO_SUFFIX) : idl_cvio.$(OBJ_SUFFIX) cvio_lib.$(OBJ_SUFFIX) \
		air_align.$(OBJ_SUFFIX) air_interface.$(OBJ_SUFFIX)
	-$(LD) $(X_LD_FLAGS) -o idl_cvio.$(SO_SUFFIX) idl_cvio.$(OBJ_SUFFIX) \
		air_align.$(OBJ_SUFFIX) air_interface.$(OBJ_SUFFIX) \
		-L.. -L../AIR5.0 -lRPW $(LIBS)

#
#	Place the DLM in the IDL directory and the apps in my bin dir
#
install : $(MANFILES)
	-mkdir ../bin
	-mkdir ../man
	-mkdir ../html
	cp -f *.htm ../html
	cp -f $(APPS) ../bin
	cp -f $(MANFILES) ../man
	cp -f *.pro ../bin
	cp -f idl_cvio.$(SO_SUFFIX) idl_cvio.dlm ../bin
	cd acquire ; make install

#
#	Build the library
#
cvio_lib.$(OBJ_SUFFIX) : cvio_lib.h

$(LIBCVIO) : cvio_lib.$(OBJ_SUFFIX) cvio_lib.h \
		cvio_sock.$(OBJ_SUFFIX)
	$(AR) $(ARP)$(LIBCVIO) cvio_lib.$(OBJ_SUFFIX) cvio_sock.$(OBJ_SUFFIX)

#
#	Build the command line apps
#
cvio_regress : cvio_regress.$(OBJ_SUFFIX) $(LIBCVIO) regress_parse.$(OBJ_SUFFIX) 75_base.$(OBJ_SUFFIX) 274.$(OBJ_SUFFIX)
	$(CC) $(C_FLAGS) cvio_regress.$(OBJ_SUFFIX) regress_parse.$(OBJ_SUFFIX) -o $@ $(LIBS) 75_base.$(OBJ_SUFFIX) 274.$(OBJ_SUFFIX)

cvio_impulse : cvio_impulse.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_impulse.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_build : cvio_build.o $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_build.o -$(OBJ_SUFFIX) $@ $(LIBS)

cvio_cp : cvio_cp.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_cp.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_info : cvio_info.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_info.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_ls : cvio_ls.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_ls.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_mk : cvio_mk.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_mk.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_rm : cvio_rm.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_rm.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_cat : cvio_cat.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_cat.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_attr : cvio_attr.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_attr.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_sample : cvio_sample.$(OBJ_SUFFIX) $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_sample.$(OBJ_SUFFIX) -o $@ $(LIBS)

cvio_rtip_pretimed_image_tag: cvio_rtip_pretimed_image_tag.c $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_rtip_pretimed_image_tag.c -o $@ $(LIBS)

cvio_image_tag : cvio_image_tag.c $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_image_tag.c -o $@ $(LIBS)

cvio_align : cvio_align.c $(LIBCVIO) air_interface.$(OBJ_SUFFIX)
	$(CC) $(C_FLAGS) cvio_align.c -o $@ air_interface.$(OBJ_SUFFIX) \
	-L.. -L../AIR5.0 -lRPW $(LIBS)

cvio_volumize : cvio_volumize.c $(LIBCVIO)
	$(CC) $(C_FLAGS) cvio_volumize.c -o $@ $(LIBS)

cvio_math : cvio_math.c $(LIBCVIO) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX) cvio_dynamic.$(OBJ_SUFFIX)
	$(CC) $(C_FLAGS) cvio_math.c -o $@ $(LIBS) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX) cvio_dynamic.$(OBJ_SUFFIX)

#
#	Dependent man pages
#
cvio_cleanup.3 : cvio_init.3
	cp -f cvio_init.3 cvio_cleanup.3
cvio_close.3 : cvio_open.3
	cp -f cvio_open.3 cvio_close.3
cvio_delete.3 : cvio_create.3
	cp -f cvio_create.3 cvio_delete.3
cvio_seek.3 : cvio_tell.3
	cp -f cvio_tell.3 cvio_seek.3
cvio_has_writer.3 : cvio_tell.3
	cp -f cvio_tell.3 cvio_has_writer.3
cvio_max_length.3 : cvio_tell.3
	cp -f cvio_tell.3 cvio_max_length.3
cvio_read_next_samples.3 : cvio_read_samples.3
	cp -f cvio_read_samples.3 cvio_read_next_samples.3
cvio_add_samples.3 : cvio_read_samples.3
	cp -f cvio_read_samples.3 cvio_add_samples.3
cvio_setattribute.3 : cvio_getattribute.3
	cp -f cvio_getattribute.3 cvio_setattribute.3
cvio_getattriblist.3 : cvio_getattribute.3
	cp -f cvio_getattribute.3 cvio_getattriblist.3

#
#	acquire code
#
$(ACQUIRE) :
	cd acquire ; make

#
#	remove the objs
#
clean : 
	rm -f idl_cvio.$(SO_SUFFIX) *.$(OBJ_SUFFIX) \
		so_locations $(LIBCVIO) $(APPS) $(MANCLEAN) *.htm
	cd acquire ; make clean

#
#	repack the sources
#
tar : $(FILES)
	rm -f idl_cvio.tar idl_cvio.tar.gz
	tar cvf idl_cvio.tar $(FILES)
	gzip idl_cvio.tar

#
#	convert man pages to html
#
html : $(MANFILES)
	@list='$(MANFILES)'; \
	 for i in $$list; do \
		groff -man -Thtml $$i > $$i.htm ; \
	 done

