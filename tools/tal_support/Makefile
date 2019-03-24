# 
#	Makefile for tal_xxx programs
#
# $Id: Makefile 2054 2006-12-13 19:38:23Z rjfrank $
#
#
#  Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski, M.D.
# 
#                 University of Iowa, Department of Neurology,
#                 Human Neuroanatomy and Neuroimaging Lab
#                 University of Iowa, Image Analysis Facility
# 
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. All modifications to the source code must be clearly marked as
#     such.  Binary redistributions based on modified source code
#     must be clearly marked as modified versions in the documentation
#     and/or other materials provided with the distribution.
#  4. All advertising materials mentioning features or use of this software
#     must display the following acknowledgment:
#       This product includes software developed by Randall Frank, Thomas
#       Grabowski and other unpaid contributors.
#  5. The names of Randall Frank or Thomas Grabowski may not be used to
#     endorse or promote products derived from this software without 
#     specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.
# 
#	Requires an ANSI C compiler and a Fortran compiler (for tal_regress 
#		and tal_pca)
#
#	Select the compiler flags and library options and type:
#
#	make all - build the executables
#	make install - place the executables into the system
#	make tar - build a tar file of the source code (talsrc.tgz)
#	make bindist - build tar file of the binary executables (talXXXbin.tgz)
#
#	make man - generate processed man pages in man.src
#	make man_install - install man pages (see man.src/Makefile first)
#
# Compilation flags:
#	Standard ANSI C stuff plus:
#	-DBV_SHMEM_SUPPORT if you have the brainvox shared memory libs
#	-DSHMEM_SUPPORT if you have the Unix shared memory support
#

VERSION=15

SHELL=/bin/sh
DEBUG=-O2
FLAGS= 
GUI_FLAGS=
GUI_LIBS=-lglut -lGL -lGLU -lX11 -lXext -lXmu -lXt -lXi -lpthread
GUI_PROGS=
BINDIST=tal_bin
MANDIST=tal_man
LIBS=-lm
OBJ_SUFFIX=o
EXE_SUFFIX=
LIB_SUFFIX=a
AR=ar -r
ARP=

ifeq ($(OSTYPE),WIN32)
CC=cl
FLAGS=-Gs -nologo -DWIN32 -DUSE_NRC -DBV_SHMEM_SUPPORT
GUI_FLAGS=
GUI_LIBS=glut32.lib glu32.lib opengl32.lib 
GUI_PROGS=tal_vbin.exe
LIBS=kernel32.lib
OBJ_SUFFIX=obj
EXE_SUFFIX=.exe
LIB_SUFFIX=lib
AR=lib /nologo
ARP=/OUT:
endif

#
#	Example Brainvox interfaces.
#	
BV_SHMEM_OBJS=../../../brainvox/trunk/libs/shared.$(OBJ_SUFFIX) \
	../../../brainvox/trunk/libs/tal_interface.$(OBJ_SUFFIX) \
	../../../brainvox/trunk/libs/io_help.$(OBJ_SUFFIX) \
	../../../brainvox/trunk/libs/mesg_box.$(OBJ_SUFFIX) \
	../../../brainvox/trunk/libs/match.$(OBJ_SUFFIX)
#
# Select exec and library locations:
#
#	Where tal_math will find its archive file (See the end of tal_math.c)
#
ifeq ($(OSTYPE),WIN32)
TAL_SUPPORT_DIR="C:\\Program Files\\U of Iowa LCN\\Brainvox\\bin"
else
ifeq ($(OSTYPE),darwin)
TAL_SUPPORT_DIR=/Applications/Brainvox.app/bin
else
TAL_SUPPORT_DIR=/usr/local/bin
endif
endif
#
#	Where do you want the executables to go?
#
INSTALL_DIR=$(TAL_SUPPORT_DIR)
#
#	Use the following if you are using the Absoft Linux f77 compiler
#	instead of f2c
#.f.o:
#	$(F77) $(FFLAGS) -c $<
#FFLAGS = -N9 -f
#LIBS = -lf77 -lm
#

.SUFFIXES: .obj .c

.c.obj:
	$(CC) /c $(CFLAGS) /Fo$*.obj $*.c


all :
	@echo "OSTYPE detected: "`uname`
	@case `uname` in \
		"SunOS" ) make allapps OSTYPE=solaris \
			"FLAGS=-DSHMEM_SUPPORT -DUSE_NRC" \
			"LIBS=-lM77 -lF77 -lm" ;;\
		"IRIX" )  make allapps OSTYPE=irix \
			"FLAGS=-DSHMEM_SUPPORT -DBV_SHMEM_SUPPORT -DUSE_NRC" \
			"GUI_PROGS=tal_vbin" ;;\
		"IRIX64" )  make allapps OSTYPE=irix \
			"GUI_FLAGS=-L../../ext_libs -I../../ext_libs" \
			"GUI_PROGS=tal_vbin" ;;\
		"Darwin" ) make allapps OSTYPE=darwin \
			"FLAGS=-DSHMEM_SUPPORT -DBV_SHMEM_SUPPORT -DUSE_NRC -I/usr/X11R6/include" \
			"LIBS=-lpthread -lm" \
			"GUI_PROGS=tal_vbin" \
			"GUI_FLAGS=-L/usr/X11R6/lib" ;;\
		"Linux" ) make allapps OSTYPE=linux \
			"FLAGS=-Wall -mcpu=pentium4 -DSHMEM_SUPPORT -DBV_SHMEM_SUPPORT -DUSE_NRC" \
			"LIBS=-lpthread -lm" \
			"GUI_PROGS=tal_vbin" \
			"GUI_FLAGS=-L/usr/X11R6/lib" ;;\
		"HP-UX" ) make allapps OSTYPE=hpux \
			"FFLAGS = -O +ppu" \
			"FLAGS=-DSHMEM_SUPPORT -Aa -D_INCLUDE_POSIX_SOURCE -D_INCLUDE_XOPEN_SOURCE -DUSE_NRC" ;;\
		"CYGWIN_NT-5.1" ) make allapps OSTYPE=WIN32 \
			;;\
		*) make allapps ;;\
	esac 
#
######################################################################
#	End of configuration options  
#	(You should not need to change things below here)
######################################################################
#
# Zlib support (use intrinsic gzip library instead of gzip via popen)
#
ZFLAGS = -DUSE_ZLIB 
ZFILES =adler32.c compress.c crc32.c gzio.c deflate.c trees.c \
        zutil.c inflate.c infblock.c inftrees.c infcodes.c infutil.c \
        inffast.c uncompr.c 
ZHFILES=deflate.h inffast.h inftrees.h trees.h zutil.h \
        zconf.h zlib.h infblock.h infcodes.h infutil.h inffixed.h 
ZLIBS  =adler32.$(OBJ_SUFFIX) compress.$(OBJ_SUFFIX) \
        crc32.$(OBJ_SUFFIX) gzio.$(OBJ_SUFFIX) \
        deflate.$(OBJ_SUFFIX) trees.$(OBJ_SUFFIX) \
        zutil.$(OBJ_SUFFIX) inflate.$(OBJ_SUFFIX) \
        infblock.$(OBJ_SUFFIX) inftrees.$(OBJ_SUFFIX) \
        infcodes.$(OBJ_SUFFIX) infutil.$(OBJ_SUFFIX) \
        inffast.$(OBJ_SUFFIX) uncompr.$(OBJ_SUFFIX) z_compress.$(OBJ_SUFFIX)

#
# Define CFLAGS (especially for tal_math)
#
CFLAGS = $(DEBUG) $(FLAGS) $(ZFLAGS) $(GUI_FLAGS)
#
#
# Sorry, I can't distribute these (Numerical Recipes in C)
# If you have permission to use these sources, place them in this
# directory and rebuild.
#
NRC_SRC = \
	nrc.c covsrt.c mrqmin.c mrqcof.c gaussj.c nrutil.c

# C Source code files
CCSRC = \
build_3dskel.c      conversion.c        convert_analyze.c   expressions.c \
fileio.c            iso_contour.c       macro.c             macro2c.c     \
make_isotab.c       mrots.c             name_changer.c      polyfill_c.c  \
regress_parse.c     skeleton2d.c        skeleton3d.c        stats.c       \
opt_search.c        label_comp.c \
tal_CDA.c           tal_centroid.c      tal_cluster.c       tal_cutting.c \
tal_euler.c         tal_histcheck.c     tal_math.c          tal_math_link.c \
tal_merge.c         tal_normalize.c     tal_probe.c         tal_rad_sample.c \
tal_regress.c       tal_rerandom.c      tal_resample.c      tal_roi_sample.c \
tal_skel_edm.c      tal_smooth.c        tal_spatial.c       tal_sphere.c     \
tal_standardize.c   tal_stat.c          tal_median.c        tal_fft.c     \
fftn.c              worsley.c           tal_remask.c	    autotrace.c \
tal_mrinorm.c	    tal_surface.c	crop_volume.c       tal_resize.c \
roi_utils.c         tal_ls.c		tal_vn_interface.c  regress_one.c \
tal_rename.c	    tal_pca.c		tal_ortho.c	    tal_optimal.c \
tal_roi2mesh.c      geom_view_io.c      tal_mcubes.c	    iso_trim.c \
tal_thinmesh.c	    tal_trace.c	        tal_shmem.c       tal_shm_interface.c \
tal_vbin.c          274.c               75_base.c           svd.c \
decimate.c          tal_warp.c		tal_gwcsf.c	    tal_nrc.c \
tal_prandom.c       tal_pts2vol.c	tal_rft.c	    tal_stat_link.c \
glm.c               nifti1.c            tal_nifti.c

# Fortran Source code
F77SRC = \
274.f               75_base.f        	svd.f

# Prototypes
HDRS = \
analyze_db.h      macro2protos.h    macroprotos.h \
proto.h           stats_inc.h       tal_regress.h \
autotrace.h	  crop_volume.h     roi_utils.h       fftn.h        \
label_comp.h	  opt_search.h      geom_view_io.h    tal_shm_interface.h \
polyfill.h	  svd.h		    as274.h           skeleton.h \
f2c.h 		  glm.h             nifti1.h

# Extra files
MISC = \
Makefile          mkcommandlines    man.src           tal_histo \
tal_histo2d       tal_means 	    as274_fc          fftn      \
readme.txt	  copying.txt	    zlib	      Makefile.msc \
f2c

#
#	make a list for tar
#
FILES = $(CCSRC) $(F77SRC) $(HDRS) $(MISC) 

#
#	define some useful object file groups
#
NETWORK_OBJS = $(BV_SHMEM_OBJS) tal_shm_interface.$(OBJ_SUFFIX)
BASEOBJS = fileio.$(OBJ_SUFFIX) name_changer.$(OBJ_SUFFIX) nifti1.$(OBJ_SUFFIX) \
           roi_utils.$(OBJ_SUFFIX) polyfill_c.$(OBJ_SUFFIX) $(NETWORK_OBJS) \
           $(ZLIBS)

#
#	Define dependancies for each program
#
OBJECTS1 = tal_resample.$(OBJ_SUFFIX) conversion.$(OBJ_SUFFIX) $(BASEOBJS)
PROG1 = tal_resample$(EXE_SUFFIX)

OBJECTS2 = tal_normalize.$(OBJ_SUFFIX) $(BASEOBJS)
PROG2 = tal_normalize$(EXE_SUFFIX)

OBJECTS3 = tal_merge.$(OBJ_SUFFIX) $(BASEOBJS)
PROG3 = tal_merge$(EXE_SUFFIX)

OBJECTS4 = tal_smooth.$(OBJ_SUFFIX) $(BASEOBJS) 
PROG4 = tal_smooth$(EXE_SUFFIX)

OBJECTS5 = tal_CDA.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROG5 = tal_CDA$(EXE_SUFFIX)

OBJECTS6 = tal_centroid.$(OBJ_SUFFIX) $(BASEOBJS)
PROG6 = tal_centroid$(EXE_SUFFIX)

OBJECTS7 = tal_cutting.$(OBJ_SUFFIX) $(BASEOBJS)
PROG7 = tal_cutting$(EXE_SUFFIX)

OBJECTS8 = tal_roi_sample.$(OBJ_SUFFIX) $(BASEOBJS)
PROG8 = tal_roi_sample$(EXE_SUFFIX)

OBJECTS9 = convert_analyze.$(OBJ_SUFFIX) $(BASEOBJS)
PROG9 = analyze2raw$(EXE_SUFFIX)

OBJECTSA = tal_probe.$(OBJ_SUFFIX) $(BASEOBJS)
PROGA = tal_probe$(EXE_SUFFIX)

TMLIB = libtal_math.$(LIB_SUFFIX)
OBJECTSB = tal_math_link.$(OBJ_SUFFIX) $(TMLIB)
PROGB = tal_math$(EXE_SUFFIX)

OBJECTSC = tal_histcheck.$(OBJ_SUFFIX) stats.$(OBJ_SUFFIX) $(BASEOBJS)
PROGC = tal_histcheck$(EXE_SUFFIX)

TSLIB = libtal_stat.$(LIB_SUFFIX)
OBJECTSD = tal_stat_link.$(OBJ_SUFFIX) $(TSLIB)
PROGD = tal_stat$(EXE_SUFFIX)

OBJECTSE = tal_cluster.$(OBJ_SUFFIX) $(BASEOBJS)
PROGE = tal_cluster$(EXE_SUFFIX)

OBJECTSF = tal_euler.$(OBJ_SUFFIX) $(BASEOBJS)
PROGF = tal_euler$(EXE_SUFFIX)

OBJECTSG = tal_sphere.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROGG = tal_sphere$(EXE_SUFFIX)

WORSLEY = worsley.$(OBJ_SUFFIX) stats.$(OBJ_SUFFIX) $(BASEOBJS)
PROG_WORSLEY = worsley$(EXE_SUFFIX)

OBJECTSH = tal_regress.$(OBJ_SUFFIX) regress_parse.$(OBJ_SUFFIX) regress_one.$(OBJ_SUFFIX) $(BASEOBJS) \
	274.$(OBJ_SUFFIX) 75_base.$(OBJ_SUFFIX)
PROGH = tal_regress$(EXE_SUFFIX)

OBJECTSI = tal_skel_edm.$(OBJ_SUFFIX) $(BASEOBJS) skeleton3d.$(OBJ_SUFFIX) skeleton2d.$(OBJ_SUFFIX) label_comp.$(OBJ_SUFFIX)
PROGI = tal_edm$(EXE_SUFFIX)

OBJECTSJ = tal_skel_edm.$(OBJ_SUFFIX) $(BASEOBJS) skeleton3d.$(OBJ_SUFFIX) skeleton2d.$(OBJ_SUFFIX) label_comp.$(OBJ_SUFFIX)
PROGJ = tal_skel$(EXE_SUFFIX)

OBJECTSZ = tal_skel_edm.$(OBJ_SUFFIX) $(BASEOBJS) skeleton3d.$(OBJ_SUFFIX) skeleton2d.$(OBJ_SUFFIX) label_comp.$(OBJ_SUFFIX)
PROGZ = tal_label$(EXE_SUFFIX)

OBJECTSK = tal_rad_sample.$(OBJ_SUFFIX) $(BASEOBJS)
PROGK = tal_rad_sample$(EXE_SUFFIX)

OBJECTSL = tal_standardize.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROGL = tal_standardize$(EXE_SUFFIX)

OBJECTSM = tal_rerandom.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROGM = tal_rerandom$(EXE_SUFFIX)

OBJECTSN = tal_spatial.$(OBJ_SUFFIX) stats.$(OBJ_SUFFIX) $(BASEOBJS)
PROGN = tal_spatial$(EXE_SUFFIX)

OBJECTSO = tal_surface.$(OBJ_SUFFIX) $(BASEOBJS) iso_contour.$(OBJ_SUFFIX)
PROGO = tal_surface$(EXE_SUFFIX)

OBJECTSP = tal_remask.$(OBJ_SUFFIX) $(BASEOBJS) autotrace.$(OBJ_SUFFIX)
PROGP = tal_remask$(EXE_SUFFIX)

OBJECTSQ = tal_mrinorm.$(OBJ_SUFFIX) $(BASEOBJS) 274.$(OBJ_SUFFIX) 75_base.$(OBJ_SUFFIX)
PROGQ = tal_mrinorm$(EXE_SUFFIX)

OBJECTSR = tal_resize.$(OBJ_SUFFIX) $(BASEOBJS) crop_volume.$(OBJ_SUFFIX)
PROGR = tal_resize$(EXE_SUFFIX)

OBJECTST = tal_ls.$(OBJ_SUFFIX) $(BASEOBJS)
PROGT = tal_ls$(EXE_SUFFIX)

OBJECTSU = tal_fft.$(OBJ_SUFFIX) fftn.$(OBJ_SUFFIX) $(BASEOBJS)
PROGU = tal_fft$(EXE_SUFFIX)

OBJECTSV = tal_median.$(OBJ_SUFFIX) stats.$(OBJ_SUFFIX) $(BASEOBJS)
PROGV = tal_median$(EXE_SUFFIX)

OBJECTSW = tal_rename.$(OBJ_SUFFIX) $(BASEOBJS)
PROGW = tal_rename$(EXE_SUFFIX)

OBJECTSX = tal_pca.$(OBJ_SUFFIX) svd.$(OBJ_SUFFIX) $(BASEOBJS)
PROGX = tal_pca$(EXE_SUFFIX)

OBJECTSY = tal_ortho.$(OBJ_SUFFIX) $(BASEOBJS)
PROGY = tal_ortho$(EXE_SUFFIX)

OBJECTSAA = tal_roi2mesh.$(OBJ_SUFFIX) $(BASEOBJS) geom_view_io.$(OBJ_SUFFIX) glm.$(OBJ_SUFFIX)
PROGAA = tal_roi2mesh$(EXE_SUFFIX)

OBJECTSAB = tal_optimal.$(OBJ_SUFFIX) $(BASEOBJS) opt_search.$(OBJ_SUFFIX)
PROGAB = tal_optimal$(EXE_SUFFIX)

OBJECTSAC = tal_mcubes.$(OBJ_SUFFIX) $(BASEOBJS) iso_contour.$(OBJ_SUFFIX) geom_view_io.$(OBJ_SUFFIX) glm.$(OBJ_SUFFIX)
PROGAC = tal_mcubes$(EXE_SUFFIX)

OBJECTSAD = tal_thinmesh.$(OBJ_SUFFIX) $(BASEOBJS) iso_trim.$(OBJ_SUFFIX) geom_view_io.$(OBJ_SUFFIX) decimate.$(OBJ_SUFFIX) \
	glm.$(OBJ_SUFFIX)
PROGAD = tal_thinmesh$(EXE_SUFFIX)

OBJECTSAE = tal_trace.$(OBJ_SUFFIX) $(BASEOBJS) autotrace.$(OBJ_SUFFIX)
PROGAE = tal_trace$(EXE_SUFFIX)

OBJECTSAF = tal_shmem.$(OBJ_SUFFIX) $(BASEOBJS)
PROGAF = tal_rmshm$(EXE_SUFFIX)

OBJECTSAG = tal_shmem.$(OBJ_SUFFIX) $(BASEOBJS)
PROGAG = tal_mkshm$(EXE_SUFFIX)

OBJECTSAH = tal_warp.$(OBJ_SUFFIX) $(BASEOBJS)
PROGAH = tal_warp$(EXE_SUFFIX)

OBJECTSAI = tal_gwcsf.$(OBJ_SUFFIX) $(BASEOBJS) 274.$(OBJ_SUFFIX) 75_base.$(OBJ_SUFFIX) skeleton3d.$(OBJ_SUFFIX) \
	autotrace.$(OBJ_SUFFIX) tal_nrc.$(OBJ_SUFFIX)
PROGAI = tal_gwcsf$(EXE_SUFFIX)

OBJECTSAJ = tal_prandom.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROGAJ = tal_prandom$(EXE_SUFFIX)

OBJECTSAK = tal_pts2vol.$(OBJ_SUFFIX) $(BASEOBJS)
PROGAK = tal_pts2vol$(EXE_SUFFIX)

OBJECTSAL = tal_rft.$(OBJ_SUFFIX) $(BASEOBJS) stats.$(OBJ_SUFFIX)
PROGAL = tal_rft$(EXE_SUFFIX)

OBJECTSAM = tal_nifti.$(OBJ_SUFFIX) $(BASEOBJS)
PROGAM = tal_nifti$(EXE_SUFFIX)

OBJECTS_GUI_A = tal_vbin.$(OBJ_SUFFIX) $(BASEOBJS)
PROG_GUI_A = tal_vbin$(EXE_SUFFIX)

#
#	build lists of targets
#

PROGS = $(PROG1)  $(PROG2)  $(PROG3)  $(PROG4)  $(PROG5)  $(PROG6)  $(PROG7) \
	$(PROG8)  $(PROG9)  $(PROGA)  $(PROGB)  $(PROGC)  $(PROGD)  $(PROGE) \
	$(PROGF)  $(PROGG)  $(PROGH)  $(PROGI)  $(PROGJ)  $(PROGK) \
	$(PROGL)  $(PROGM)  $(PROGN)  $(PROGO)  $(PROGP)  $(PROGQ) \
	$(PROGR)  $(PROGT)  $(PROGU)  $(PROGV)  $(PROGW)  $(PROGX) \
	$(PROGY)  $(PROGZ)  $(PROGAA) $(PROGAB) $(PROGAC) $(PROGAD) \
	$(PROGAE) $(PROGAF) $(PROGAG) $(PROGAH) $(PROGAI) $(PROGAJ) \
        $(PROGAK) $(PROGAL) $(PROGAM) \
	$(PROG_WORSLEY)

EXES = $(PROGS) raw2analyze$(EXE_SUFFIX) tal_spherep$(EXE_SUFFIX) $(GUI_PROGS)

EXTRAS = tal_histo tal_histo2d tal_means

#
#	define targets
#
allapps: $(EXES) commandlines man

fileio.c : gzio.$(OBJ_SUFFIX)
	touch fileio.c

install:
	-chmod o+x $(EXES) tal_vbin$(EXE_SUFFIX)
	-chmod o+r $(EXES) tal_vbin$(EXE_SUFFIX) libtal_*
	-cp $(EXES) tal_vbin$(EXE_SUFFIX) $(INSTALL_DIR)
	cp libtal_* $(TAL_SUPPORT_DIR)
	$(MAKE) man_install

ifeq ($(OSTYPE),darwin)
PMD=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/
PM=$(PMD)/PackageMaker
endif

bindist:
	-mkdir bin
	chmod o+x bin
	chmod o+r bin
	-chmod o+x $(EXES) tal_vbin$(EXE_SUFFIX)
	-chmod o+r $(EXES) tal_vbin$(EXE_SUFFIX) libtal_*
	-cp $(EXES) tal_vbin$(EXE_SUFFIX) libtal_* bin
	-cp readme.txt bin
	tar cvf $(BINDIST).tar bin
	gzip $(BINDIST).tar
	mv $(BINDIST).tar.gz $(BINDIST).tgz
	-rm -r bin
	-mkdir man1
	-cp man.src/*.1 man1
	tar cvf $(MANDIST).tar man1
	gzip $(MANDIST).tar
	mv $(MANDIST).tar.gz $(MANDIST).tgz
	-rm -r man1
ifeq ($(OSTYPE),darwin)
	-mkdir -p ./tal_dist/Package_Root/Applications/Brainvox.app/bin
	-mkdir -p ./tal_dist/Package_Root/Applications/Brainvox.app/lib
	-mkdir -p ./tal_dist/Package_Root/Applications/Brainvox.app/relnotes
	-mkdir -p ./tal_dist/Package_Root/Applications/Brainvox.app/man/man1
	-install -m 755 $(EXES) tal_vbin$(EXE_SUFFIX) \
		./tal_dist/Package_Root/Applications/Brainvox.app/bin
	-install -m 755 libtal_* \
		./tal_dist/Package_Root/Applications/Brainvox.app/lib
	-install -m 755 man.src/*.1 \
		./tal_dist/Package_Root/Applications/Brainvox.app/man/man1
	-install -m 755 readme.txt \
		./tal_dist/Package_Root/Applications/Brainvox.app/relnotes
	$(PM) -build -p ./tal_dist.pkg -f ./tal_dist/Package_Root \
		-i Info.plist -d Description.plist -ds -v
	-install -m 644 ReadMe.rtf  ./tal_dist.pkg/Contents/Resources
	-install -m 644 License.rtf ./tal_dist.pkg/Contents/Resources
	-install -m 644 Welcome.rtf ./tal_dist.pkg/Contents/Resources
	hdiutil create -srcfolder tal_dist.pkg tal_programs \
		-volname tal_programs -ov
	gzip tal_programs.dmg
	-rm -rf tal_dist tal_dist.pkg
endif

tar:
	cd man.src; make clean
	cd zlib; make clean
	rm -rf talsrc_no_nrc.tgz
	tar cvf talsrc.tar $(FILES)
	gzip talsrc.tar
	mv talsrc.tar.gz talsrc_no_nrc.tgz

tar_nrc:
	cd man.src; make clean
	cd zlib; make clean
	tar cvf talsrc.tar $(FILES) $(NRC_SRC)
	gzip talsrc.tar
	mv talsrc.tar.gz talsrc.tgz

clean:
	cd man.src; make clean
	rm -f *.o *.obj *.exe core $(EXES) $(TMLIB) $(TSLIB) \
		$(GUI_PROGS) $(PROG_GUI_A) $(ZFILES) $(ZHFILES) \
		skel3d_etable.h iso_tab.h *.lib

commandlines: $(PROGS)
	./mkcommandlines $(PROGS) $(GUI_PROGS)

man:
	cd man.src; make

man_install:
	cd man.src; make install

#
#	a few special cases...
#
skeleton3d.$(OBJ_SUFFIX) : skel3d_etable.h   skeleton3d.c

skel3d_etable.h : build_3dskel.c
	$(CC) $(CFLAGS) build_3dskel.c
ifeq ($(OSTYPE),WIN32)
	./build_3dskel.exe
	rm -f build_3dskel.exe
else
	./a.out
	rm -f a.out
endif

iso_contour.c : iso_tab.h
	touch iso_contour.c

iso_tab.h : make_isotab.c
	$(CC) $(CFLAGS) make_isotab.c
ifeq ($(OSTYPE),WIN32)
	./make_isotab.exe > iso_tab.h
	rm -f make_isotab.exe
else
	./a.out > iso_tab.h
	rm -f a.out
endif

crop_volume.$(OBJ_SUFFIX) : crop_volume.c crop_volume.h

#
#	build each program
#
$(PROG_WORSLEY) : $(WORSLEY)
	$(CC) $(CFLAGS) $(WORSLEY) -o $(PROG_WORSLEY) $(LIBS)
$(PROG1) : $(OBJECTS1)
	$(CC) $(CFLAGS) $(OBJECTS1) -o $(PROG1) $(LIBS)
$(PROG2) : $(OBJECTS2)
	$(CC) $(CFLAGS) $(OBJECTS2) -o $(PROG2) $(LIBS)
$(PROG3) : $(OBJECTS3)
	$(CC) $(CFLAGS) $(OBJECTS3) -o $(PROG3) $(LIBS)
$(PROG4) : $(OBJECTS4)
	$(CC) $(CFLAGS) $(OBJECTS4) -o $(PROG4) $(LIBS)
$(PROG5) : $(OBJECTS5)
	$(CC) $(CFLAGS) $(OBJECTS5) -o $(PROG5) $(LIBS)
$(PROG6) : $(OBJECTS6)
	$(CC) $(CFLAGS) $(OBJECTS6) -o $(PROG6) $(LIBS)
$(PROG7) : $(OBJECTS7)
	$(CC) $(CFLAGS) $(OBJECTS7) -o $(PROG7) $(LIBS)
$(PROG8) : $(OBJECTS8)
	$(CC) $(CFLAGS) $(OBJECTS8) -o $(PROG8) $(LIBS)
$(PROG9) : $(OBJECTS9)
	$(CC) $(CFLAGS) $(OBJECTS9) -o $(PROG9) $(LIBS)
	cp $(PROG9) raw2analyze$(EXE_SUFFIX)
$(PROGA) : $(OBJECTSA)
	$(CC) $(CFLAGS) $(OBJECTSA) -o $(PROGA) $(LIBS)
$(PROGB) : $(OBJECTSB)
	$(CC) $(CFLAGS) $(OBJECTSB) -o $(PROGB) $(LIBS)
$(PROGC) : $(OBJECTSC)
	$(CC) $(CFLAGS) $(OBJECTSC) -o $(PROGC) $(LIBS)
$(PROGD) : $(OBJECTSD)
	$(CC) $(CFLAGS) $(OBJECTSD) -o $(PROGD) $(LIBS)
$(PROGE) : $(OBJECTSE)
	$(CC) $(CFLAGS) $(OBJECTSE) -o $(PROGE) $(LIBS)
$(PROGF) : $(OBJECTSF)
	$(CC) $(CFLAGS) $(OBJECTSF) -o $(PROGF) $(LIBS)
$(PROGG) : $(OBJECTSG)
	$(CC) $(CFLAGS) $(OBJECTSG) -o $(PROGG) $(LIBS)
	cp $(PROGG) tal_spherep$(EXE_SUFFIX)
$(PROGH) : $(OBJECTSH)
	$(CC) $(CFLAGS) $(OBJECTSH) -o $(PROGH) $(LIBS)
$(PROGI) : $(OBJECTSI)
	$(CC) $(CFLAGS) $(OBJECTSI) -o $(PROGI) $(LIBS)
$(PROGJ) : $(OBJECTSJ)
	$(CC) $(CFLAGS) $(OBJECTSJ) -o $(PROGJ) $(LIBS)
$(PROGK) : $(OBJECTSK)
	$(CC) $(CFLAGS) $(OBJECTSK) -o $(PROGK) $(LIBS)
$(PROGL) : $(OBJECTSL)
	$(CC) $(CFLAGS) $(OBJECTSL) -o $(PROGL) $(LIBS)
$(PROGM) : $(OBJECTSM)
	$(CC) $(CFLAGS) $(OBJECTSM) -o $(PROGM) $(LIBS)
$(PROGN) : $(OBJECTSN)
	$(CC) $(CFLAGS) $(OBJECTSN) -o $(PROGN) $(LIBS)
$(PROGO) : $(OBJECTSO)
	$(CC) $(CFLAGS) $(OBJECTSO) -o $(PROGO) $(LIBS)
$(PROGP) : $(OBJECTSP)
	$(CC) $(CFLAGS) $(OBJECTSP) -o $(PROGP) $(LIBS)
$(PROGQ) : $(OBJECTSQ)
	$(CC) $(CFLAGS) $(OBJECTSQ) -o $(PROGQ) $(LIBS)
$(PROGR) : $(OBJECTSR)
	$(CC) $(CFLAGS) $(OBJECTSR) -o $(PROGR) $(LIBS)
$(PROGT) : $(OBJECTST)
	$(CC) $(CFLAGS) $(OBJECTST) -o $(PROGT) $(LIBS)
$(PROGU) : $(OBJECTSU)
	$(CC) $(CFLAGS) $(OBJECTSU) -o $(PROGU) $(LIBS)
$(PROGV) : $(OBJECTSV)
	$(CC) $(CFLAGS) $(OBJECTSV) -o $(PROGV) $(LIBS)
$(PROGW) : $(OBJECTSW)
	$(CC) $(CFLAGS) $(OBJECTSW) -o $(PROGW) $(LIBS)
$(PROGX) : $(OBJECTSX)
	$(CC) $(CFLAGS) $(OBJECTSX) -o $(PROGX) $(LIBS)
$(PROGY) : $(OBJECTSY)
	$(CC) $(CFLAGS) $(OBJECTSY) -o $(PROGY) $(LIBS)
$(PROGZ) : $(OBJECTSZ)
	$(CC) $(CFLAGS) $(OBJECTSZ) -o $(PROGZ) $(LIBS)
$(PROGAA) : $(OBJECTSAA)
	$(CC) $(CFLAGS) $(OBJECTSAA) -o $(PROGAA) $(LIBS)
$(PROGAB) : $(OBJECTSAB)
	$(CC) $(CFLAGS) $(OBJECTSAB) -o $(PROGAB) $(LIBS)
$(PROGAC) : $(OBJECTSAC)
	$(CC) $(CFLAGS) $(OBJECTSAC) -o $(PROGAC) $(LIBS)
$(PROGAD) : $(OBJECTSAD)
	$(CC) $(CFLAGS) $(OBJECTSAD) -o $(PROGAD) $(LIBS)
$(PROGAE) : $(OBJECTSAE)
	$(CC) $(CFLAGS) $(OBJECTSAE) -o $(PROGAE) $(LIBS)
$(PROGAF) : $(OBJECTSAF)
	$(CC) $(CFLAGS) $(OBJECTSAF) -o $(PROGAF) $(LIBS)
$(PROGAG) : $(OBJECTSAG)
	$(CC) $(CFLAGS) $(OBJECTSAG) -o $(PROGAG) $(LIBS)
$(PROGAH) : $(OBJECTSAH)
	$(CC) $(CFLAGS) $(OBJECTSAH) -o $(PROGAH) $(LIBS)
$(PROGAI) : $(OBJECTSAI)
	$(CC) $(CFLAGS) $(OBJECTSAI) -o $(PROGAI) $(LIBS)
$(PROGAJ) : $(OBJECTSAJ)
	$(CC) $(CFLAGS) $(OBJECTSAJ) -o $(PROGAJ) $(LIBS)
$(PROGAK) : $(OBJECTSAK)
	$(CC) $(CFLAGS) $(OBJECTSAK) -o $(PROGAK) $(LIBS)
$(PROGAL) : $(OBJECTSAL)
	$(CC) $(CFLAGS) $(OBJECTSAL) -o $(PROGAL) $(LIBS)
$(PROGAM) : $(OBJECTSAM)
	$(CC) $(CFLAGS) $(OBJECTSAM) -o $(PROGAM) $(LIBS)

$(PROG_GUI_A) : $(OBJECTS_GUI_A)
	$(CC) $(CFLAGS) $(GUI_FLAGS) $(OBJECTS_GUI_A) -o $(PROG_GUI_A) \
		$(LIBS) $(GUI_LIBS)
#
#  zlib files...
#
$(ZFILES) :
	cp zlib_113/* .
#
#	build the tal_math library (for command line recompiling)
#
$(TMLIB) : tal_math.$(OBJ_SUFFIX) $(BASEOBJS) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX)
	$(AR) $(ARP)$(TMLIB) tal_math.$(OBJ_SUFFIX) $(BASEOBJS) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX)
#
#	build the tal_stat library (for command line recompiling)
#
$(TSLIB) : tal_stat.$(OBJ_SUFFIX) $(BASEOBJS) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX)
	$(AR) $(ARP)$(TSLIB) tal_stat.$(OBJ_SUFFIX) $(BASEOBJS) macro.$(OBJ_SUFFIX) macro2c.$(OBJ_SUFFIX)

