#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox                
#
#       To disable the MP_FIT options define the CPP macro NO_MP_FIT
#
include ../config

FILES = vtuif.f brainvox.f reaper.c

OBJECTS = vtuif.$(OBJ_SUFFIX) brainvox.$(OBJ_SUFFIX) reaper.$(OBJ_SUFFIX)

POBJECTS = vtuif.$(OBJ_SUFFIX) brainvox_prefs.$(OBJ_SUFFIX)

all: brainvox$(EXE_SUFFIX) brainvox_prefs$(EXE_SUFFIX)

ifeq ($(OS),WIN32)
FCL = link /out:brainvox$(EXE_SUFFIX)
RESSRC = brainvox.rc
RES =   $(RESSRC:.rc=.res)
%.res:  %.rc
	rc /v /x $<
else
RES =
FCL = $(FC) -o brainvox$(EXE_SUFFIX)
endif

brainvox_prefs$(EXE_SUFFIX) : $(POBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS)  $(POBJECTS) -o $@ \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

brainvox$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB) $(RES)
	$(FCL) $(FFLAGS) $(OBJECTS) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB) $(RES)

$(OBJECTS) : ../libs/vt_inc.f 

vtuif.$(OBJ_SUFFIX) : vtuif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) brainvox core vtuif.f vtuif.inc brainvox_prefs

install:
	cp brainvox$(EXE_SUFFIX) ../bin
	cp brainvox_prefs$(EXE_SUFFIX) ../bin
ifeq ($(OS),WIN32)
	cp  ../../../tools/trunk/tal_support/*$(EXE_SUFFIX) ../bin
	cp  ../../../tools/trunk/tal_support/*.$(LIB_SUFFIX) ../bin
	-mkdir ../bin/man
	cp  ../../../tools/trunk/tal_support/man.src/*.txt ../bin/man
endif
