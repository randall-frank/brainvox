=============
CVIO & others
=============


This distribution includes source code for several packages developed
by Randall Frank and other contributors.  The following packages are
included:


`CVIO <https://github.com/randall-frank/brainvox/tree/master/tools/cvio>`_
--------------------------------------------------------------------------

This archive contains the source to the University of Iowa, 
Hospitals and Clinics, Department of Neurology, CVIO library.  This
library is designed to allow for dynamic I/O operations on up to
3D data samples.  The API is inherently time aware and allows for
controlled access to the time varying data.  An IDL wrapper is
provided to allow a user to call the entire CVIO library from IDL.
Note: the shared memory interface is not supported under Windows.


`idl_tools <https://github.com/randall-frank/brainvox/tree/master/tools/idl_tools>`_
------------------------------------------------------------------------------------

A collection of IDL add-on tools (DLM) to expand the
functionality of IDL.

- TWAIN interface for image acquisition
- Shared memory interface
- Dynamic direct DLL calls on Windows
- Windows DirectSound interface
- Raw sockets interface
- Mesh processing (thinning, union, intersection)
- gzip interface


`Regression <https://github.com/randall-frank/brainvox/tree/master/tools/regression>`_
--------------------------------------------------------------------------------------

IDL DLM implementing an incremental regression
system with support for various test statistics.  The algorithms used
are based on the functions provided by Applied Statistics routine 274
with some additions.  The IDL wrapper includes a parallel interface,
making it possible to run hundreds if not thousands of regressions
in parallel.



Building
--------

Most of this software targets the Unix environment, but many of the
sources (particularly the IDL interfaces) have been ported to 
Windows as well.  Look for Visual C++ project files for those
packages that have been ported.

The software has been tested under Irix and Linux.  Most people can
just type 'make' at the top level to build all the files.  Some
editing of individual lower level Makefiles may be necessary
and comments to aid in this are present in several Makefiles.  
Wherever possible, the user modifiable parameters are at the
top of the Makefiles.


Contributors
------------

Many people have contributed to this software archive and we would
like to recognize some of them here.

- Hanna Damasio, M.D.
- Carl Kice Brown
- Steven Beck, Boyd Knosp, and other personnel at the University of Iowa Image Analysis Facility
- Chris Smyser
- Lizann Bolinger
- Sonya Mehta
- Brent Eaton

