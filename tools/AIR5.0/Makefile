#
#	Build wrapper for AIR5 for the Univ of Iowa
# $Id: Makefile,v 1.4 2002/03/11 20:37:06 rjfrank Exp $
#

AIRDIR = AIR5.0

all:
	cd $(AIRDIR); ./configure; make
	cp $(AIRDIR)/src/libRPW.a .
	cp $(AIRDIR)/src/AIR.h $(AIRDIR)/src/config*.h .

clean:
	cd $(AIRDIR); make clean
	rm -f AIR.h libRPW.a

install:
	cp $(AIRDIR)/src/libRPW.a .
	cp $(AIRDIR)/src/AIR.h $(AIRDIR)/src/config*.h .
