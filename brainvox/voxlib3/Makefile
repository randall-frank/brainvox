#
#	Real makefile begins here
#
include ../config

CFLAGS = $(CDEBUG) -I. -DGLCODE -I../include $(XINC) $(GINC) -D$(OS)
PLIB = libvox3.$(LIB_SUFFIX)

PFILES = \
vl_p_matrix.c   vl_polygon.h    vl_polygons.c   vl_putil.h \
vl_p_render.c   vl_p_fileio.c	vl_softfont.c   vl_pixmap.c \
vl_tmasks.h     vl_p_picking.c  vl_pickfont.c

POBJECTS = \
vl_p_matrix.$(OBJ_SUFFIX) vl_polygons.$(OBJ_SUFFIX) vl_p_render.$(OBJ_SUFFIX) \
vl_p_fileio.$(OBJ_SUFFIX) vl_softfont.$(OBJ_SUFFIX) vl_pixmap.$(OBJ_SUFFIX) \
vl_p_picking.$(OBJ_SUFFIX) vl_pickfont.$(OBJ_SUFFIX) 

FILES =	\
vl_file_io.c        vl_name_ch.c        vl_data.c           vl_image.c \
vl_init.c           vl_limits.c         vl_lookups.c        vl_plane.c \
vl_render.c         vl_rotations.c      vl_speed.c          vl_trans.c \
vl_calcmatrix.c     vl_gtxrender.c      vl_pirender.c       vl_interp.c \
vl_makeplane.c      vl_drawcube.c       vl_getmatrix.c      vl_private.h \
vl_squeeze.c        vl_filter.c		vl_callbacks.c      vl_postlight.c \
vl_dither.c         voxel.h 		vl_util.h 	    Makefile \
colorin.h    	    colortbl.inc        vl_matrix.c \
vl_voxel_renderers.h vl_postlight.h     vl_shadows.c        $(PFILES)  \
vl_m_render.c       vl_m_pirender.c     vl_m_ncpus.c        vl_parallel.c \
vl_parallel.h	    vl_cliprect.c	vl_m_pirend16.c     vl_m_pirend24.c \
vl_vol2render.c	    iafdefs.h           vl_3dtexrender.c    vl_pbuffer.c \
vl_pbuffer.h        vtkmarchingcubescases.h   vl_agl.c      vl_stubs.c

OBJECTS = \
vl_file_io.$(OBJ_SUFFIX) vl_name_ch.$(OBJ_SUFFIX) vl_data.$(OBJ_SUFFIX) \
vl_image.$(OBJ_SUFFIX) vl_init.$(OBJ_SUFFIX) vl_limits.$(OBJ_SUFFIX) \
vl_lookups.$(OBJ_SUFFIX) vl_plane.$(OBJ_SUFFIX) vl_render.$(OBJ_SUFFIX) \
vl_rotations.$(OBJ_SUFFIX) vl_speed.$(OBJ_SUFFIX) vl_trans.$(OBJ_SUFFIX) \
vl_calcmatrix.$(OBJ_SUFFIX) vl_pirender.$(OBJ_SUFFIX) vl_pbuffer.$(OBJ_SUFFIX) \
vl_interp.$(OBJ_SUFFIX) vl_makeplane.$(OBJ_SUFFIX) vl_drawcube.$(OBJ_SUFFIX) \
vl_getmatrix.$(OBJ_SUFFIX) vl_squeeze.$(OBJ_SUFFIX) vl_filter.$(OBJ_SUFFIX) \
vl_dither.$(OBJ_SUFFIX) vl_postlight.$(OBJ_SUFFIX) vl_callbacks.$(OBJ_SUFFIX) \
vl_matrix.$(OBJ_SUFFIX) vl_shadows.$(OBJ_SUFFIX) vl_m_render.$(OBJ_SUFFIX) \
vl_m_pirender.$(OBJ_SUFFIX) vl_m_ncpus.$(OBJ_SUFFIX) vl_parallel.$(OBJ_SUFFIX) \
vl_cliprect.$(OBJ_SUFFIX) vl_m_pirend16.$(OBJ_SUFFIX) \
vl_m_pirend24.$(OBJ_SUFFIX) vl_vol2render.$(OBJ_SUFFIX) \
vl_3dtexrender.$(OBJ_SUFFIX) vl_agl.$(OBJ_SUFFIX) vl_stubs.$(OBJ_SUFFIX)

all: $(PLIB)

$(PLIB) : $(OBJECTS) $(POBJECTS)
	$(AR) $(ARP)$@ $(OBJECTS) $(POBJECTS)
	$(RANLIB) $@

$(OBJECTS) : voxel.h vl_util.h

$(POBJECTS) : voxel.h vl_util.h vl_polygon.h vl_putil.h vl_postlight.h \
	vl_tmasks.h vl_pbuffer.h

backup:
	cp $(FILES) back_up

clean:
	-rm -f *.o *.a *.obj *.lib

install:
	$(RANLIB) $(PLIB)
	-cp $(PLIB) ../ext_libs
	-cp voxel.h ../ext_libs
	-cp vl_util.h ../ext_libs
	-cp vl_polygon.h ../ext_libs
	-cp iafdefs.h   ../ext_libs
	-cp vl_putil.h  ../ext_libs

