#
#	Makefile for the man pages
#	$Id: Makefile 1934 2006-09-05 00:50:29Z rjfrank $
#

MAN_DIR=/usr/local/man/man1
GZMAN_DIR=/usr/local/man/man1

FILES=\
tal_cda.man      tal_centroid.man tal_cluster.man   tal_cutting.man tal_edm.man \
tal_euler.man    tal_fft.man      tal_gwcsf.man     tal_label.man   tal_ls.man \
tal_math.man     tal_mcubes.man   tal_median.man    tal_merge.man   tal_mkshm.man \
tal_mrinorm.man  tal_nifti.man    tal_normalize.man tal_optimal.man tal_ortho.man \
tal_pca.man   tal_probe.man    tal_programs.man  tal_regress.man tal_remask.man \
tal_rename.man   tal_rerandom.man tal_resample.man  tal_resize.man  tal_rmshm.man \
tal_roi2mesh.man tal_roi_samp.man tal_skel.man   tal_smooth.man  tal_spatial.man \
tal_sphere.man   tal_standard.man tal_stat.man   tal_surface.man tal_thinmesh.man \
tal_trace.man    tal_vbin.man     tal_warp.man

SHELL=/bin/sh

.SUFFIXES: .1 .man

.1.man:
	-nroff -man $*.1 > $*.man
	-cp $*.man $*.txt

all: $(FILES)

installz: $(FILES)
	for d in $(FILES); do \
		x=`basename $$d .man`; \
		cp $$d $$x; \
		compress $$x; \
		mv $$x.Z $(MAN_DIR); \
		chmod 0644 $(MAN_DIR)/$$x.Z; \
	done

install: $(FILES)
	for d in $(FILES); do \
		x=`basename $$d .man`; \
		cp $$d $$x.tmp; \
		gzip -f $$x.tmp; \
		mv $$x.tmp.gz $(GZMAN_DIR)/$$x.1.gz; \
		chmod 0644 $(GZMAN_DIR)/$$x.1.gz; \
	done

clean:
	rm -f *.man
