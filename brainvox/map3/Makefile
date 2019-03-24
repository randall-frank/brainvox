#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_regions        
#
include ../config

FILES = map3uif.r map3.f map3_file_io.f compute_map.f

OBJECTS = map3uif.$(OBJ_SUFFIX) map3.$(OBJ_SUFFIX) map3_file_io.$(OBJ_SUFFIX) compute_map.$(OBJ_SUFFIX)

TRACE_OBJS = ../trace/vt_arb_mode_io.$(OBJ_SUFFIX)

brainvox_map3$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) -o brainvox_map3$(EXE_SUFFIX) $(TRACE_OBJS) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f

map3uif.$(OBJ_SUFFIX) : map3uif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core brainvox_map3$(EXE_SUFFIX) map3uif.f map3uif.inc

install:
	cp brainvox_map3$(EXE_SUFFIX) ../bin
