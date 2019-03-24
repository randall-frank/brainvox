#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_calc
#
include ../config

PROG = calc

FILES = $(PROG)uif.r $(PROG).f calcarea.f intersectarea.f \
	paintarea.f outputdata.f calc_volumes.f outputdata_v2.f

OBJECTS = $(PROG)uif.$(OBJ_SUFFIX) $(PROG).$(OBJ_SUFFIX) calcarea.$(OBJ_SUFFIX) intersectarea.$(OBJ_SUFFIX) \
	paintarea.$(OBJ_SUFFIX) outputdata.$(OBJ_SUFFIX) calc_volumes.$(OBJ_SUFFIX) outputdata_v2.$(OBJ_SUFFIX)

brainvox_$(PROG)$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) -o brainvox_$(PROG) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f 

$(PROG)uif.$(OBJ_SUFFIX) : $(PROG)uif.r
	../bin/rmaker$(EXE_SUFFIX) $(PROG)uif.r $(PROG)uif.f $(PROG)uif.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) brainvox_$(PROG)$(EXE_SUFFIX) core $(PROG)uif.f $(PROG)uif.inc

install:
	cp brainvox_$(PROG)$(EXE_SUFFIX) ../bin
