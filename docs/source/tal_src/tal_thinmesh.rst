============
tal_thinmesh
============


NAME
====

tal_thinmesh - Performs vertex reduction on a polygon mesh file.

SYNOPSIS
========

tal_thinmesh [options] inputfile outputfile

DESCRIPTION
===========

This program reads a polygon file (in tal\_ Geomview format) and will
reduce the number of polygons in the surface by removing verticies of
polygons which are nearly coplanar and representing the patch with 2
fewer triangles. The inputfile and outputfile (or '-' for stdin/stdout)
are in Wavefront OBJ format and consists of a single triangle mesh. The
files must have the "# TAL_PROGRAMS Geomview file 1.0" header comment as
well.

It is not always possible for this algorithm to succeed as it will not
remove verticies which are part of an outer boundary. In such cases, as
many verticies as can be removed are.

Options:

-t(thin)
   This option specifies the fraction of the verticies to remove (0.0 to
   1.0). The default is 0.0 (no thinning).

-g
   Use the Geomview NOFF file format for input and output.

SEE ALSO
========

tal_programs, geomview, tal_surface, tal_roi2mesh, tal_mcubes

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
