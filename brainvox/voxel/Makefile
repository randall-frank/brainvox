#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for brainvox_voxel
#
#	To disable the MP_FIT options define the CPP macro NO_MP_FIT
#	To disable the taliarach options define the CPP macro NO_TAL_SPACE
#	To disable the Howard options define the CPP macro NO_DSPACE
#
DEFINES=-DNO_DSPACE 
include ../config

PROG = voxel

FILES = $(PROG)uif.r $(PROG).f check_extents.f get_2dimage.f spaceconvert.f\
	paintpoints.f delpoints.f save_rgb.f draw_image.f addpoints.f\
	draw_grid.f resample.c cutcalcs.c getroipnt.f measurements.f \
	setupdialog.f calc_fp.f calc_up.f point_io.f calc_sa.f tal_space.f\
	tal_overlays.f cube_stuff.f cube_cstuff.c rename_pset.f fit_plane.c\
	erasepoints.f mpfit_settingsio.f lighting_model.f windows.f\
	makelabels.c AIR_support.c texture_control.f \
	texture_update.f mappings.c clip_volume.c texture_clip.f \
	cursor_3d.f voxel_init.f mouse_procs.f autotrace.c movie.f \
	disk_work.f mp_fit.f macro.c macro.h script.c

OBJECTS = $(PROG)uif.$(OBJ_SUFFIX) check_extents.$(OBJ_SUFFIX) get_2dimage.$(OBJ_SUFFIX) spaceconvert.$(OBJ_SUFFIX)\
	paintpoints.$(OBJ_SUFFIX) delpoints.$(OBJ_SUFFIX) save_rgb.$(OBJ_SUFFIX) draw_image.$(OBJ_SUFFIX) addpoints.$(OBJ_SUFFIX)\
	draw_grid.$(OBJ_SUFFIX) resample.$(OBJ_SUFFIX) cutcalcs.$(OBJ_SUFFIX) getroipnt.$(OBJ_SUFFIX) measurements.$(OBJ_SUFFIX) \
	setupdialog.$(OBJ_SUFFIX) calc_fp.$(OBJ_SUFFIX) calc_up.$(OBJ_SUFFIX) point_io.$(OBJ_SUFFIX) calc_sa.$(OBJ_SUFFIX) tal_space.$(OBJ_SUFFIX)\
	tal_overlays.$(OBJ_SUFFIX) cube_stuff.$(OBJ_SUFFIX) cube_cstuff.$(OBJ_SUFFIX) rename_pset.$(OBJ_SUFFIX) fit_plane.$(OBJ_SUFFIX)\
	erasepoints.$(OBJ_SUFFIX) mpfit_settingsio.$(OBJ_SUFFIX) lighting_model.$(OBJ_SUFFIX) windows.$(OBJ_SUFFIX) \
	makelabels.$(OBJ_SUFFIX) AIR_support.$(OBJ_SUFFIX) texture_control.$(OBJ_SUFFIX) \
	texture_update.$(OBJ_SUFFIX) macro.$(OBJ_SUFFIX) mappings.$(OBJ_SUFFIX) clip_volume.$(OBJ_SUFFIX) texture_clip.$(OBJ_SUFFIX) \
	cursor_3d.$(OBJ_SUFFIX) voxel_init.$(OBJ_SUFFIX) mouse_procs.$(OBJ_SUFFIX) autotrace.$(OBJ_SUFFIX)  movie.$(OBJ_SUFFIX) \
	disk_work.$(OBJ_SUFFIX) $(PROG).$(OBJ_SUFFIX) \
	script.$(OBJ_SUFFIX)

all: brainvox_$(PROG)$(EXE_SUFFIX) brainvox_mp_fit$(EXE_SUFFIX)


brainvox_$(PROG)$(EXE_SUFFIX) : $(OBJECTS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS)  $(OBJECTS) -o brainvox_$(PROG)$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

MP_FIT_OBJS = mpfit_settingsio.$(OBJ_SUFFIX) tal_space.$(OBJ_SUFFIX) calc_fp.$(OBJ_SUFFIX) calc_up.$(OBJ_SUFFIX) mp_fit.$(OBJ_SUFFIX) \
	point_io.$(OBJ_SUFFIX) resample.$(OBJ_SUFFIX) cutcalcs.$(OBJ_SUFFIX) spaceconvert.$(OBJ_SUFFIX) AIR_support.$(OBJ_SUFFIX) 

brainvox_mp_fit$(EXE_SUFFIX) : $(MP_FIT_OBJS) ../libs/libvtrace.$(LIB_SUFFIX) $(LCL_LIB)
	$(FC) $(FFLAGS)  $(MP_FIT_OBJS) -o brainvox_mp_fit$(EXE_SUFFIX) \
		$(BLIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

$(OBJECTS) : ../libs/vt_inc.f

voxel.$(OBJ_SUFFIX) : ../libs/vt_inc.f rotsdef.inc windows.inc

windows.$(OBJ_SUFFIX) : windows.inc

calc_fp.$(OBJ_SUFFIX) : rotsdef.inc

calc_up.$(OBJ_SUFFIX) : rotsdef.inc windows.inc

setupdialog.$(OBJ_SUFFIX) : ../libs/vt_inc.f rotsdef.inc windows.inc

paintpoints.$(OBJ_SUFFIX) : windows.inc

erasepoints.$(OBJ_SUFFIX) : windows.inc

cube_stuff.$(OBJ_SUFFIX) : windows.inc

addpoints.$(OBJ_SUFFIX) : windows.inc

$(PROG)uif.$(OBJ_SUFFIX) : $(PROG)uif.r
	../bin/rmaker$(EXE_SUFFIX) $*.r $*.f $*.inc
	$(FC) $(FFLAGS) -c $*.f

clean:
	-rm -f *.$(OBJ_SUFFIX) core brainvox_$(PROG)$(EXE_SUFFIX) $(PROG)uif.f brainvox_mp_fit$(EXE_SUFFIX) \
	$(PROG)uif.inc

install:
	cp brainvox_$(PROG)$(EXE_SUFFIX) ../bin
	cp brainvox_mp_fit$(EXE_SUFFIX) ../bin

