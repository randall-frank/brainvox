#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_regions        
#
include ../config

FILES = regionsuif.r regions.f

OBJECTS = regionsuif.$(OBJ_SUFFIX) regions.$(OBJ_SUFFIX) 

brainvox_regions$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) -o brainvox_regions$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f 

regionsuif.$(OBJ_SUFFIX) : regionsuif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core brainvox_regions$(EXE_SUFFIX) regionsuif.f regionsuif.inc

install:
	cp brainvox_regions$(EXE_SUFFIX) ../bin
