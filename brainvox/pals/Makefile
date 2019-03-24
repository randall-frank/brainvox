#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_pals 
#
include ../config

FILES = palsuif.r pals.f histo.f validate.c colorchange.c fixed_cmaps.c \
	pal_tools.f

OBJECTS = palsuif.$(OBJ_SUFFIX) pals.$(OBJ_SUFFIX) histo.$(OBJ_SUFFIX) validate.$(OBJ_SUFFIX) colorchange.$(OBJ_SUFFIX) fixed_cmaps.$(OBJ_SUFFIX) \
	pal_tools.$(OBJ_SUFFIX)

brainvox_pals$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) -o brainvox_pals$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f 

palsuif.$(OBJ_SUFFIX) : palsuif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core brainvox_pals$(EXE_SUFFIX) palsuif.f palsuif.inc

install:
	cp brainvox_pals$(EXE_SUFFIX) ../bin
