#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_trace                
#
include ../config

PROG = trace
PROG2 = brainvox_arb_roi_paint
PROG3 = brainvox_gamma

VOBJECTS = ../voxel/resample.$(OBJ_SUFFIX) ../voxel/cutcalcs.$(OBJ_SUFFIX) ../voxel/paintpoints.$(OBJ_SUFFIX)

FILES2 = brainvox_arb_roi_paint.f

OBJECTS2 = brainvox_arb_roi_paint.$(OBJ_SUFFIX) vt_arb_mode.$(OBJ_SUFFIX) vt_arb_mode_io.$(OBJ_SUFFIX) \
	vt_read_roi.$(OBJ_SUFFIX) \
	vt_read_images.$(OBJ_SUFFIX) vt_read_roi_disk.$(OBJ_SUFFIX) virt.$(OBJ_SUFFIX) readall.$(OBJ_SUFFIX) \
	trace_exit.$(OBJ_SUFFIX) vt_texture.$(OBJ_SUFFIX) save256.$(OBJ_SUFFIX) \
	$(VOBJECTS)

OBJECTS3 = set_gamma.$(OBJ_SUFFIX)

FILES =	$(FILES2) trace.f vt_init.f setlists.f swapstates.f \
	vt_draw_image.f setbysliders.f drawtraces.f vt_read_roi.f\
	vt_write_roi.f addline.f doedit.f bumpsliders.f virt.f\
	vt_read_roi_disk.f traceuif.f draw_grid.f \
	autotrace.f vt_thinroi.f trace_exit.f vt_read_images.f\
	readall.f cutout.f vt_closehemi.f save_rgb.f calcarea.f \
	vt_texture.f vt_arb_mode.f split_hemis.f search_hemis.c \
	opt_search2.c vt_arb_mode_io.f save256.c

OBJECTS = traceuif.$(OBJ_SUFFIX) trace.$(OBJ_SUFFIX) vt_init.$(OBJ_SUFFIX) setlists.$(OBJ_SUFFIX) swapstates.$(OBJ_SUFFIX) \
	vt_draw_image.$(OBJ_SUFFIX) setbysliders.$(OBJ_SUFFIX) drawtraces.$(OBJ_SUFFIX) vt_read_roi.$(OBJ_SUFFIX)\
	vt_write_roi.$(OBJ_SUFFIX) addline.$(OBJ_SUFFIX) doedit.$(OBJ_SUFFIX) bumpsliders.$(OBJ_SUFFIX) virt.$(OBJ_SUFFIX)\
	vt_read_roi_disk.$(OBJ_SUFFIX) draw_grid.$(OBJ_SUFFIX) \
	autotrace.$(OBJ_SUFFIX) vt_thinroi.$(OBJ_SUFFIX) trace_exit.$(OBJ_SUFFIX) vt_read_images.$(OBJ_SUFFIX)\
	readall.$(OBJ_SUFFIX) cutout.$(OBJ_SUFFIX) vt_closehemi.$(OBJ_SUFFIX) save_rgb.$(OBJ_SUFFIX) calcarea.$(OBJ_SUFFIX) \
	vt_texture.$(OBJ_SUFFIX) vt_arb_mode.$(OBJ_SUFFIX) split_hemis.$(OBJ_SUFFIX) search_hemis.$(OBJ_SUFFIX) \
	opt_search2.$(OBJ_SUFFIX) vt_arb_mode_io.$(OBJ_SUFFIX) save256.$(OBJ_SUFFIX) \
	$(VOBJECTS)

all : brainvox_$(PROG)$(EXE_SUFFIX) $(PROG2)$(EXE_SUFFIX) $(PROG3)$(EXE_SUFFIX)

brainvox_$(PROG)$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS) -o brainvox_$(PROG)$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(PROG2)$(EXE_SUFFIX) : $(OBJECTS2) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS) $(OBJECTS2) -o $(PROG2)$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(PROG3)$(EXE_SUFFIX) : $(OBJECTS3)
	$(CC) -O $(OBJECTS3) -o $(PROG3)$(EXE_SUFFIX) -lm
	chmod 4711 $(PROG3)

$(OBJECTS) : tr_inc.f ../libs/vt_inc.f

$(PROG)uif.$(OBJ_SUFFIX) : $(PROG)uif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core $(PROG2)$(EXE_SUFFIX) $(PROG3)$(EXE_SUFFIX) brainvox_$(PROG)$(EXE_SUFFIX) $(PROG)uif.f \
	$(PROG)uif.inc

install:
	cp $(PROG2)$(EXE_SUFFIX) $(PROG3)$(EXE_SUFFIX) brainvox_$(PROG)$(EXE_SUFFIX) ../bin

