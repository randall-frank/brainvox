---------------------------------------------------------------------------
Univ of Iowa, Neuroimaging Laboratory Software Distribution   Release 1.0.0
$Id: readme.txt 260 2001-05-18 22:23:14Z rjfrank $
---------------------------------------------------------------------------

Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski, M.D.

University of Iowa, Department of Neurology, Human Neuroanatomy and 
Neuroimaging Lab
University of Iowa, Image Analysis Facility

        See the file "COPYING" for further remarks.

This distribution includes source code for several packages developed
by Randall Frank and other contributors.  The following packages are
included:


tal : System of command line tools for the manipulation and analysis of
volume datasets.  This sowftare was developed in support of the Univ of
Iowa, Neuroimaging Laboratory and Univ of Iowa Image Analysis Facility.

cvio : System for handling streaming, time stamped data.  Basic I/O
libraries and tools for real-time analysis (including regression) and
included.  This sowftare was developed in support of the Univ of
Iowa, Neuroimaging Laboratory.  Interfaces to IDL (Research Systems,
Inc, Boulder, CO) are included for many functions.

idl_tools : A collection of IDL add-on tools (DLM) to expand the
functionality of IDL.


Most of this software targets the Unix environment, but many of the
sources (particularly the IDL interfaces) have been ported to 
Windows as well.  Look for Visual C++ project files for those
packages that have been ported.

The software has been tested under Irix and Linux.  Most people can
just type 'make' at the top level to build all the files.  Some
editting of individual lower level Makefiles may be necessary
and comments to aid in this are present in several Makefiles.  
Wherever possible, the user modifiable parameters are at the
top of the Makefiles.

Many people have contributed to this software archive and we would
like to recognize some of them here.

Contributors:
	Hanna Damasio, M.D.
	Carl Kice Brown
	Steven Beck, Boyd Knosp, and other personnel at the
		University of Iowa Image Analysis Facility
	Chris Smyser
	Lizann Bolinger
	Sonya Mehta
	Brent Eaton

	many others...


