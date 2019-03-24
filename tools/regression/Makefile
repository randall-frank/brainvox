#
# Unix makefile for CVIO_REG DLM.
# $Id: Makefile 1250 2005-09-16 15:51:42Z dforeman $
#

#       The following macro might need to be edited if the location
#       of IDL is not the standard location on your system.
#IDL_DIR = /usr/local/rsi/idl


# The following macros establish general defaults. They are overridden
# in the "all" rule as needed to satisfy a given platform's requirements.
CC              = cc -g
C_FLAGS         = -I$(IDL_DIR)/external -c $(CFLAGS)
LD              = ld
SHELL           = /bin/sh
X_CFLAGS        =
X_LD_FLAGS      =
SO_EXT          =so

APPS            = regression_test
MANFILES	= reg_lib.3 \
	          reg_create.3 reg_destroy.3 \
	          reg_add_obs.3 reg_remove_obs.3 \
	          reg_calc_ss.3 \
	          reg_calc_coef.3 
#
#	derived man page files...
#
MANCLEAN	= reg_destroy.3 reg_remove_obs.3

LIBS            = -L. -lm
CFLAGS          = -I.

FILES = cvio_reg.c cvio_reg.dlm cvio_reg.export cvio_reg.def \
	Makefile regression.h regression.c regression_test.c 274.c as274.h \
	test_reg.pro 274.f copying.txt readme.txt cvio_reg.dsp cvio_reg.dsw \
	$(MANFILES)

.c.o :
	$(CC) $(C_FLAGS) $(X_CFLAGS) $*.c

# The following is the default entry point. This section will determine
# what system we are on and call make again with the correct flags set.

all : $(APPS)
	@echo "OS type detected: "`uname`
	@if [ $${#IDL_DIR} = 0 ]; then \
	        echo "IDL_DIR variable must be set for DLMs" ; \
	else \
	  case `uname` in \
           "SunOS") if [ -d /proc ]; then \
                        make cvio_reg \
                        "X_CFLAGS=-K pic" \
                        "X_LD_FLAGS=-B symbolic -z nodefs -G" ; \
                    else \
                      make cvio_reg "CC=acc" \
                        "X_CFLAGS= -pic" \
                        "X_LD_FLAGS= -assert pure-text" ; \
                    fi  \
                ;; \
           "AIX") make cvio_reg \
                        "LD=cc" \
                        "SO_EXT=a" \
                        "X_CFLAGS=-qro -qroconst" \
                        "X_LD_FLAGS= -bM:SRE -bnoentry -btextro -bE:cvio_reg.export -bI:$(IDL_DIR)/external/idl.export" ;;\
           "HP-UX") make cvio_reg  \
                        "SO_EXT= sl" \
                        "X_CFLAGS=+z -Aa -D_HPUX_SOURCE" \
                        "X_LD_FLAGS= -b -Brestricted +s" ;; \
           "IRIX" )  make cvio_reg \
                       "X_LD_FLAGS=-Bsymbolic -shared -all -rdata_shared" ;;\
           "IRIX64" )  make cvio_reg \
                       "X_LD_FLAGS=-Bsymbolic -shared -all -rdata_shared" ;;\
           "OSF1" )  make cvio_reg \
                        "X_CFLAGS=-float" \
                        "X_LD_FLAGS=-expect_unresolved '*' -shared -all" ;;\
           "Linux" ) make cvio_reg \
                        "X_CFLAGS= -fPIC" \
                        "X_LD_FLAGS= -shared -Bsymbolic --warn-once -noinhibit-exec " ;; \
	   "Darwin" ) make idl_cvio \
	   		"X_CFLAGS= -no-cpp-precomp -dynamic -fPIC -fno-common -D_REENTRANT" \
			"CC=gcc" \
			"LD=gcc" \
			"X_LD_FLAGS= -bundle -flat_namespace -undefined suppress" ;; \
           *) echo "This system is not supported" ;; \
	  esac \
	fi
#
# The following is the entry point for the recursive build
#
cvio_reg : cvio_reg.$(SO_EXT)
	@date

cvio_reg.$(SO_EXT) : cvio_reg.o regression.o 274.o
	-$(LD) $(X_LD_FLAGS) -o cvio_reg.$(SO_EXT) cvio_reg.o  \
	regression.o 274.o $(LIBS) 

#
#	Dependent man pages
#
reg_destroy.3 : reg_create.3
	cp -f reg_create.3 reg_destroy.3
reg_remove_obs.3 : reg_add_obs.3
	cp -f reg_add_obs.3 reg_remove_obs.3

#
#       Place the DLM in the IDL directory and the apps in my bin dir
#
install: $(MANFILES)
	-mkdir ../bin
	-mkdir ../man
	cp cvio_reg.$(SO_EXT) cvio_reg.dlm ../bin
	cp regression_test ../bin
	cp *.pro ../bin
	cp $(MANFILES) ../man

#
#       remove the objs
#
clean :
	rm -f *.o cvio_reg.so cvio_reg.sl cvio_reg.a \
		so_locations $(APPS) $(MANCLEAN)

#
#       repack the sources
#
tar : $(FILES)
	rm -f cvio_reg.tar cvio_reg.tar.gz
	tar cvf cvio_reg.tar $(FILES)
	gzip cvio_reg.tar

#
#	APPS...
#
regression_test: regression_test.o regression.o 274.o
	$(CC) $(CFLAGS) regression_test.o regression.o 274.o -o $@ -lm
