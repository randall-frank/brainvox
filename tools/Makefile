#
#	Top level Makefile for the following packages:
#
#	$Id: Makefile 1250 2005-09-16 15:51:42Z dforeman $
#
# cvio - Univ of Iowa, CVIO library and tools
# regression - progressive linear regression library
# tal_support - Univ of Iowa, tal programs (including gwcsf segmentation)
# idl_tools - Randall Frank, IDL extensions
# cvio_tools - Brent Eaton, fMRI and/or DSP apps using CVIO library
# 
# The majority of this software was written by and is Copyright
# Randall Frank and University of Iowa collaborators.
#
# Note: Some of these packages require RSI's IDL application
#

PACKAGES = AIR5.0 tal_support cvio cvio_tools regression idl_tools cvio_tools 
FILES = Makefile readme.txt copying.txt

all :
	@list='$(PACKAGES)'; \
	for i in $$list; do \
		if [ -d $$i ] ; then \
			echo "Making: $$i"; \
			(cd $$i && '$(MAKE)' ); \
		fi \
	done

install :
	-mkdir bin
	-mkdir man
	-mkdir html
	@list='$(PACKAGES)'; \
	for i in $$list; do \
		if [ -d $$i ] ; then \
			echo "Making clean: $$i"; \
			(cd $$i && '$(MAKE)' install ); \
		fi \
	done

clean :
	@list='$(PACKAGES)'; \
	for i in $$list; do \
		if [ -d $$i ] ; then \
			echo "Making clean: $$i"; \
			(cd $$i && '$(MAKE)' clean ); \
		fi \
	done
	rm -rf bin
	rm -rf man
	rm -rf html

tar:
	$(MAKE) clean
	rm -f alltools.tar alltools.tgz
	tar cvf alltools.tar $(FILES) $(PACKAGES)
	gzip alltools.tar
	mv alltools.tar.gz alltools.tgz

