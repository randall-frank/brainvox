#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for sgiuif                  
#
include ../config

LIBS = $(ULIB) $(GLIB) $(XLIB) $(FLIB) $(CLIB)

FILES =	dlg_d_box.f dlg_d_button.f dlg_d_checkbx.f dlg_d_edit.f\
	dlg_d_line.f dlg_d_menu.f dlg_d_sbar.f dlg_d_text.f\
	draw_dlg.f get_dlg.f handle_dlg.f init_dlg.f dlg_d_color.f\
	dlg_center.f dlg_inrect.f dlg_frects.f dlg_dosbar.f dlg_d_radio.f\
	dlg_cursor.f

OBJECTS = dlg_d_box.$(OBJ_SUFFIX) dlg_d_button.$(OBJ_SUFFIX) dlg_d_checkbx.$(OBJ_SUFFIX) dlg_d_edit.$(OBJ_SUFFIX)\
	dlg_d_line.$(OBJ_SUFFIX) dlg_d_menu.$(OBJ_SUFFIX) dlg_d_sbar.$(OBJ_SUFFIX) dlg_d_text.$(OBJ_SUFFIX)\
	draw_dlg.$(OBJ_SUFFIX) get_dlg.$(OBJ_SUFFIX) handle_dlg.$(OBJ_SUFFIX) init_dlg.$(OBJ_SUFFIX) dlg_d_color.$(OBJ_SUFFIX)\
	dlg_center.$(OBJ_SUFFIX) dlg_inrect.$(OBJ_SUFFIX) dlg_frects.$(OBJ_SUFFIX) dlg_dosbar.$(OBJ_SUFFIX) dlg_d_radio.$(OBJ_SUFFIX)\
	dlg_cursor.$(OBJ_SUFFIX) mallocf.$(OBJ_SUFFIX) readdir.$(OBJ_SUFFIX) fmcode.$(OBJ_SUFFIX) cursors.$(OBJ_SUFFIX)

all: libuif.$(LIB_SUFFIX) rmaker$(EXE_SUFFIX)

libuif.$(LIB_SUFFIX) : $(OBJECTS) 
	$(AR) $(ARP)libuif.$(LIB_SUFFIX) $(OBJECTS)
	$(RANLIB) $@

$(OBJECTS) : sgiuif.h sgiuif.f

mallocf.$(OBJ_SUFFIX) : mallocf.c

readdir.$(OBJ_SUFFIX) : readdir.c

fmcode.$(OBJ_SUFFIX) : fmcode.c
	$(CC) $(CFLAGS) -O -I ../include -DGL_ONLY -c fmcode.c

cursors.$(OBJ_SUFFIX) : cursors.c
	$(CC) -O -I ../include -c cursors.c

rmaker$(EXE_SUFFIX) : rmaker.$(OBJ_SUFFIX)
	$(FC) -O rmaker.$(OBJ_SUFFIX) -o rmaker$(EXE_SUFFIX) $(FLIB) 

rmaker.$(OBJ_SUFFIX) : rmaker.f sgiuif.h

mkploticon$(EXE_SUFFIX) : mkploticon.$(OBJ_SUFFIX)
	$(CC) -O -s mkploticon.$(OBJ_SUFFIX) -o mkploticon$(EXE_SUFFIX)

hello$(EXE_SUFFIX)  : hello.$(OBJ_SUFFIX) libuif.$(LIB_SUFFIX)
	$(FC) -O hello.$(OBJ_SUFFIX) -o hello$(EXE_SUFFIX) $(LIBS)

hello.$(OBJ_SUFFIX) : hello.f sgiuif.h

chello$(EXE_SUFFIX) : chello.$(OBJ_SUFFIX) libuif.$(LIB_SUFFIX) hellores.$(OBJ_SUFFIX)
	$(FC) -O chello.$(OBJ_SUFFIX) hellores.$(OBJ_SUFFIX) -o chello$(EXE_SUFFIX) $(LIBS)

hellores.f : hello.res
	./rmaker$(EXE_SUFFIX) hello.res hellores.f

chello.$(OBJ_SUFFIX) : chello.f sgiuif.h

hellores.$(OBJ_SUFFIX) : hellores.f sgiuif.h

clean:
	-rm -f *.$(OBJ_SUFFIX) *.$(LIB_SUFFIX) core hello$(EXE_SUFFIX) chello$(EXE_SUFFIX) rmaker$(EXE_SUFFIX)

install:
	-mkdir ../bin
	-mkdir ../ext_libs
	-cp rmaker$(EXE_SUFFIX) ../bin
	$(RANLIB) libuif.$(LIB_SUFFIX)
	-cp libuif.$(LIB_SUFFIX) ../ext_libs
	-cp sgiuif.h ../ext_libs

