#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_filer
#
include ../config

FILES =	filer.f fileruif.r

OBJECTS = filer.$(OBJ_SUFFIX) 

brainvox_filer$(EXE_SUFFIX) : $(OBJECTS) fileruif.$(OBJ_SUFFIX) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) fileruif.$(OBJ_SUFFIX) -o brainvox_filer$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f fileruif.$(OBJ_SUFFIX)

fileruif.$(OBJ_SUFFIX) : fileruif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core brainvox_filer$(EXE_SUFFIX) fileruif.f fileruif.inc

install:
	cp brainvox_filer$(EXE_SUFFIX) ../bin
