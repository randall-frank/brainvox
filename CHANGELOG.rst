*******
History
*******

This repository includes several different packages the release notes for each
of those packages are included after the overall package changelog

Brainvox repo release notes:
============================

v4.0.0
------
* Conversion to scons for building

v1.0.0
------
* Basic source code upload

****

Tal_programs release notes:
===========================

v1.5.2 03 Jun 06
----------------
* Fixed issues with isocontouring and polygon file I/O.

v1.5.0 27 Aug 05
----------------
* Roll-up release for patches since 01.

v1.4.0 04 Mar 01
----------------
* Added tal_rft and tal_pts2vol.
* Added new output volume to tal_warp.
* Added custom output function for tal_stat.

v1.3.0 17 Dec 00
----------------
* Added tal_prandom. 
* tal_stat gets new local stats (correlation and sum of diff squ) as
	well as improved masking functions.  
* Made interpixel spacings different in x and y.
* Added tal_gwcsf for MRI segmentation.

v1.2.9 08 Oct 00
----------------
*	Added weighted observations to tal_regress

v1.2.8 07 Jun 99
----------------
* Added tal_warp for volume vector resampling.
* Replaced the tal_thin algorithm.
* Fixed a bug in the vector cmdline parsing routine.

v1.2.7 20 Jan 99
----------------
* Added support for Windows (95/NT).
* Compression support is limited.
* Requires MS VC 6.0.

v1.2.6 22 Sep 98
----------------
* Added ATAN2() to tal_math and added 1D option to tal_fft.  
* Added support for single file volumes:
	"xxxx:0_%%%{_volhdr{_slicehdr}}{_f}" uncompressed only.

v1.2.5 12 Apr 98
----------------
* Fixed a bug in tal_stat where it would try to read
	images past the last in the stack.
* Added intrinsic zlib support (.gz files, see ZFLAGS and ZLIBS in the Makefile).  
* Added initial version of tal_vbin.
 
v1.2.4 07 Mar 98
----------------
* Cleanup of a number of codes and addition of better
	prototypes for .f functions.  This fixed a couple
	of obscure command line parsing bugs.  
* Improved some portability aspects of the code.
 
v1.2.3 27 Dec 97
----------------
* Added ROI flipping and scaling to tal_trace, removed
	them from tal_roi2mesh (they were redundant).
* tal_trace: fixed a bug in horizontal edge cases.
* tal_edm: improved speed, fixed a minor direction bug

v1.2.2 28 Sep 97
----------------
* Changed the defaults for tal_resample:-l, 
	tal_cluster:-o, and tal_stat:-k.
* Added: tal_trace, tal_mkshm, tal_rmshm (added shared 
  memory block I/O support).

v1.2.1 16 Jun 96
----------------
* Replaced the median routine, removed wrap-around,
	and added a mask volume to tal_median
* Added tal_optimal, a 3-D optimal graph search program
	(actually much more, see opt_search.*)
* Added tal_roi2mesh which converts a stack of ROIs to
	a 3D triangle mesh (Geomview format)
* Added tal_mcubes which performs volume isosurface
	detection (outputs a mesh in Geomview format)
* Added tal_thinmesh which will reduce the number
	of polygons in a mesh

v1.2.0 03 Jun 96
----------------
* Fixed a bug in tal_euler when the threshold was 0
* Added higher order resel outputs to tal_euler  

v1.1.0 06 May 96
----------------
* Added tal_label for connected components labeling
* Added support for ASCII format image files
* Documented "negative" threshold features
* Fixed a namespace bug in tal_math which prevented
	certain expressions from compiling
* The SGI binary release form is now in ELF format
	(will not run under Irix 4.x)

v1.0.0 01 Apr 96
----------------
* First public release.

****

Brainvox release notes:
=======================

v3.14
-----
* fixed two bugs in the scripting language.
* added scripting history.
* reworked filer handling of top level directories.

v3.13
-----
* support for nifti/analyze format files.
* a new scripting language has been added.

v3.12
-----
* a couple of really nasty filename handling bugs fixed.

v3.11
-----
* fixed an issue with name handling in the region editor.
* reworked MAP-3 to run under Windows.
* removed many of the "system()" calls to simplify the API.

v3.10
-----
* added some support for dir names with spaces.
* fixed more depth buffer issues.
* added support for tal_programs w/brainvox.

v3.09
-----
* there is now a Windows (XP, 32bit) version.
* fixed issues with depth buffer clipping for planes, etc.

v3.08
-----
* a large number of cosmetic changes (renaming, moving items, etc).
* fixed an issue with clipping the 3D cursor display lines.
* fixed an issue with MAP-3 on the Mac.

v3.07
-----
* added HW interpolation as an option.
* added point voxel/texture probing.

v3.06
-----
* make the arrow keys work on the Mac.
* added the ability to save the tracing images as a volume.
 
v3.05
-----
* fixed a problem that caused Brainvox to use too much CPU time.
* added a means to allow 3D cursor movement for OSX (was alt-ctrl).
* fixed OSX window resizing issue.

v3.04
-----
* fixed a bug in brainvox_calc where ill-formed ROIs crashed it.
* backup speed in trace window works now.
* all windows mapped to double buffering...

v3.03
-----
* made .gz and .Z file support intrinsic (needed for Win32).
* fixed issue with finding the arb_paint program.
* added "Preferences".
* initial cut at OSX version.
* added a "reload textures" option for use with tal_programs.
* brainvox-tal_program connectivity re-enabled.

v3.02
-----
* improved long filenames support.
* fix for 16bit lit HW rendering.
* fix for arb_mode angle issues ("hang").

v3.01
-----
* initial version with support for longer filenames.
* fixes for portability.

v3.00
-----
* This is the first major revision in several years.
* hardware accelerated volume rendering
  Requires ARB_fragment_program support and 128MB of video RAM
* mouse buttons switched to be more inline with other GUI toolkits
  "setenv BRAINVOX_SWAP_BUTTONS 0" will revert to older mapping
* brainvox no longer needs to be in the search path.  It will find
  all its tools on the fly and use absolute pathnames.
* added "drag and drop" launching in that the command line:
  brainvox /my/path/name
  will 'cd' to /my/path/name and then run brainvox.  Very
  useful for "dropping" a directory on brainvox
* added "auto-rendering"
* remapped many of the interaction buttons for consistency and added a "help" menu
* pointsets can be "shaded" when occluded (as well as being clipped)
* the speed of the tracing "backup" has had limits added
* the "quality" menu has been reworked for better options organization
* more of the code has been moved to libraries reducing replication
* updates made for new Absoft Fortran compilers
* allow voxel tracing on the "background"
  If tracing in the 3D window is done with the 'Z' key held
  down, the tracing is not clipped to the voxel surface, rather
  the depth is set explicitly to Z=0 (the center of the volume)
  Very useful when creating pointsets to be used as clips in
  the texture volume
* ALT or CTRL key while rotating cube will rotate in the plane of the display.

