---------------------------------------------------------------------------
Brainvox tal_ support programs                                Release 1.5.2
$Id: readme.txt 1790 2006-06-03 21:53:19Z rjfrank $
---------------------------------------------------------------------------

Copyright (c), 1990-2006 Randall Frank and Thomas Grabowski, M.D.

University of Iowa, Department of Neurology, Human Neuroanatomy and 
Neuroimaging Laboratory
University of Iowa, Image Analysis Facility

	See the file "copying.txt" for further remarks.

Introduction:

This archive contains a suite of programs known collectively as the
Brainvox tal_ programs.  Source code to the programs is provided  (where
possible) and a binary distribution for Silicon Graphics workstations is
provided as well.  The programs are command line orientated and can be
built on most any UNIX system that has an ANSI C compiler and a Fortran
compiler (or f2c).  They have been used on SGI, HP and Linux systems.  A
copy of gzip/gunzip is necessary to access these distributions.  Basic
support for compilation under Windows 95/NT is included as well.

Basically, the programs form an interlocking set of tools which are
generally scripted together to perform volume imaging tasks.  They were
designed for use in MRI and PET neuroimaging (there are a number of
modules specifically targeted at PET statistics), but have proven useful
in a number of other applications.  Input images are raw binary headerless
and can be in 8bit, 16bit or floating point resolutions. One image is
stored in each file.  Refer to the man page for tal_programs for further
details on common command line options between the modules (man pages may
be found in the man.src directory).


Installing the software:

This software release comes in two forms.  The first is a source code
distribution and the second is a pre-built binary distribution containing
just the executables for a system. 

The source code will normally come in a file named: talsrc.tgz or
alltools.tgz.  To build the applications, one would normally do
the following:

% gunzip -dc talsrc.tgz | tar xof -
% make
% make install

If you have a binary distribution for your system (note: binary
distributions are normally built to install at /usr/local/bin):

% cd /usr/local/bin
% gunzip -dc tal_linux_bin.tgz | tar xvof -

    The binary executables will be in a bin subdirectory.

If you are using a binary distribution, you need to move the files in the
"bin" directory to a final resting location on your system.  One file
"libtal_math.a" should be placed in "/usr/local/bin" (if you cannot use
"/usr/local/bin", you can either set the environmental variable
TAL_SUPPORT_DIR to the directory containing this file, or recompile the
tal_math executable from the source code).  

Man pages for the tal programs are installed into /usr/local/man/man1
by default.  If you wish to specify another destination, modify
the Makefile in the man.src directory to specify their destination.

The Makefile attempts to configure itself for common systems, but there
are a number of custom configurations.  See the top of the Makefile for
examples of user configurable settings.  Note: if you do not have access 
to the "Numerical  Recipies in C" source code, you need to unselect 
"-DUSE_NRC" in "Makefile"  to use some alternative routines.  Also, source 
code to support Brainvox  shared memory and VoxBlast server connections 
cannot be made publicly available at this time, so any user built programs 
will not have  these features.

The next step I recommend is to read through and/or install the man pages
for the various programs.  They are located in "man.src" and nroff source
and ASCII text versions are both included.  Install as needed on your
system.  I suggest reading the tal_programs man page first to get an
overview of the suite.

Notes:
	The support for compressed files may not work with some revisions
of the SUN operating system which buffer stdin/stdout into BUFSIZ chunks.
I am aware of the problem, but have not fixed it yet (the systems I use do
not duplicate this behavior).

Windows NT/95 users (this has not been maintained for some time, so it
	may be out of date):
	The release can be built using MicroSoft Visual C++ 6.0, using the 
following command line:
  
nmake /f makefile.msc

and

nmake /f makefile.msc install

	to install the exes.  This must first be done in the zlib directory
for compression support. For tal_vbin, the glut library must be installed
on your system.  The -c option for tal_math assumes VC 6.0 is installed and
the compiler can be accessed via 'cl'. 
  Current limitations under Windows:
    1) compress (.Z) is not supported (as popen does not work)

Updates/bugs:

I (Randall Frank) am the primary person working on this software.  If you
have a question or a problem, you can email me, but I do not know how much
support I can give you.  If you find any bugs, please let me know and I
will try to address them.  Also, I am open to suggestions for enhancements
or additional  programs.

Randall Frank
randall-frank@computer.org (email preferred)

Additional contact:

Thomas Grabowski, M.D.
thomas-grabowski@uiowa.edu (email preferred)
University of Iowa, Dept of Neurology
Human Neuroanatomy and Neuroimaging Lab
Iowa City, IA 52242-1053


Release Notes:

1.0	 1 Apr 96	First public release.

1.1	 6 May 96	Added tal_label for connected components labeling
			Added support for ASCII format image files
			Documented "negative" threshold features
			Fixed a namespace bug in tal_math which prevented
				certain expressions from compiling
			The SGI binary release form is now in ELF format
				(will not run under Irix 4.x)

1.2	 3 Jun 96	Fixed a bug in tal_euler when the threshold was 0
			Added higher order resel outputs to tal_euler

1.2.1	16 Jun 96	Replaced the median routine, removed wrap-around,
			and added a mask volume to tal_median
			Added tal_optimal, a 3-D optimal graph search program
			(actually much more, see opt_search.*)
			Added tal_roi2mesh which converts a stack of ROIs to
			a 3D triangle mesh (Geomview format)
			Added tal_mcubes which performs volume isosurface
			detection (outputs a mesh in Geomview format)
			Added tal_thinmesh which will reduce the number
			of polygons in a mesh

1.2.2	28 Sep 97	Changed the defaults for tal_resample:-l, 
			tal_cluster:-o, and tal_stat:-k.  Added: tal_trace,
			tal_mkshm, tal_rmshm (added shared memory block
			I/O support).

1.2.3	27 Dec 97	Added ROI flipping and scaling to tal_trace, removed
			them from tal_roi2mesh (they were redundant).
			tal_trace: fixed a bug in horizontal edge cases.
			tal_edm: improved speed, fixed a minor direction bug

1.2.4	 7 Mar 98	Cleanup of a number of codes and addition of better
			prototypes for .f functions.  This fixed a couple
			of obscure command line parsing bugs.  Improved
			some portability aspects of the code.

1.2.5	12 Apr 98	Fixed a bug in tal_stat where it would try to read
			images past the last in the stack.  Added intrinsic
			zlib support (.gz files, see ZFLAGS and ZLIBS in the
			Makefile).  Added initial version of tal_vbin.

1.2.6	22 Sep 98	Added ATAN2() to tal_math and added 1D option to
			tal_fft.  Added support for single file volumes:
			"xxxx:0_%%%{_volhdr{_slicehdr}}{_f}" uncompressed 
			only.

1.2.7	20 Jan 99	Added support for Windows (95/NT).
			Compression support is limited.
			Requires MS VC 6.0.

1.2.8    7 Jun 99	Added tal_warp for volume vector resampling.
			Replaced the tal_thin algorithm.  Fixed a bug
			in the vector cmdline parsing routine.

1.2.9    8 Oct 00	Added weighted observations to tal_regress

1.3.0    17 Dec 00	Added tal_prandom.  tal_stat gets new local
			stats (correlation and sum of diff squ) as
			well as improved masking functions.  Made
			interpixel spacings different in x and y.
			Added tal_gwcsf for MRI segmentation.

1.4.0	4 Mar 01	Added tal_rft and tal_pts2vol.  Added new
			output volume to tal_warp.  Added custom
			output function for tal_stat.

1.5.0   27 Aug 05     	Roll-up release for patches since 01.

1.5.2   03 Jun 06     	Fixed issues with isocontouring and ploygon
                        file I/O.

TODO:
	add support for Z dimension time series volumes (tal_stat)
	change FFT code to FFTW library 
	convert mcube to tetrahedron based codes
	tal_math: add lookup table functions
	several apps: support mask volumes
	tal_label: add 16bit/float output
	tal_conv16to8: 16bit file translation
	add support for the cvio file format
	revise the cmdline parsing for external interfaces
	new ideas for the I/O abstraction: templates w/defs+modifiers

Acknowledgments:

	The authors would like to thank the following people for their
input into this software.
	
	Hanna Damasio, M.D.
	Sonya Mehta
	Carl Kice Brown
	Steven Beck, Boyd Knosp, and other personnel at the 
		University of Iowa Image Analysis Facility


Other contributors:

There are several files in this distribution from other authors which
are not covered by the notices in the file "COPYING":

274.f: ALGORITHM AS 274.1  APPL. STATIST. (1992) VOL 41, NO. 2
75_base.f: Algorithm AS 75.2 Appl. Statist. (1974) Vol. 23, No. 3, P448
	See the directory "as274_fc" as well.

convert_analyze.c, worsley.c: Randall Frank

fftn.c: RC Singleton, Mark Olesen, John Beale
	See the directory "fftn" as well.

macro.c & macro2c.c: 
	Adapted from: Ronald Mak, 'Writing Compilers and Interpreters: An
	Applied Approach' by Randall Frank

nrc.c: 'Numerical Recipies in C' (not included in the distribution)

stats.c: Ray Toy, Jos van der Woude, the Gnuplot group, and Randall Frank

svd.f: LINPACK, G.W. STEWART, UNIVERSITY OF MARYLAND, ARGONNE NATIONAL LAB.


tal_rft.c, tal_pts2vol.c : Sonya Mehta

