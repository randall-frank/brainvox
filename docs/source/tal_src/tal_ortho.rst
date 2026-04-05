=========
tal_ortho
=========


NAME
====

tal_ortho - Performs volume orthogonal plane resampling.

SYNOPSIS
========

tal_ortho [options] inputtemp outputtemp

DESCRIPTION
===========

This program resamples an input volume into a series of images parallel
to a different orthogonal plane. It can be used to resample an axial
volume into a coronal volume for example. In addition, the program
allows for any axis to be inverted (flipped) during the operation.

INPUTTEMP is the input volume template. OUTPUTTEMP is the volume
template for the output resampled volume. The -c option is used to
define the size of the output volume (in pixels/slices). The interpixel
and interslice spacings for the input and output volumes are specified
by the -p, -s, and -r options. Note that the output volume interpixel
spacing is fixed to that of the input volume. The orientation of the
output volume axis is selected though the use of the -Dx, -Dy, and -Dz
options. The unit vector describing the output axis in terms of the
input volume axis is specified by these options. For example, if the new
X axis is to be the input volume Z axis, the -Dx option would be:
-Dx0:0:1, (mapping the new X axis on the input Z axis 0,0,1). If the new
X axis should be mapped to the inverse of the input Z axis (flipped
along the Z axis), the -Dx option would be: -Dx0:0:-1. Only orthogonal
axis parallel one of the input axis are allowed. Thus, the only valid
values to a -D option are 0, 1, and -1. In addition, two of the three
values must be 0. Sampling is done via nearest neighbor.

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 1 bytes (unsigned shorts).

-f(start)
   This specifies the slice number for the first image in the volume.
   Default:1.

-l(end)
   This specifies the slice number for the last image in the volume.
   Default:100.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using
   (useless in the current mode, but would be needed to support
   filtering).

-v
   This enables verbose mode.

-s(interslice)
   The interslice spacing of the intput volume is specified by this
   option. Default:1.

-p(interpixel)
   The interpixel spacing of the input volume is specified by this
   option. Default:1.

-r(interslice)
   The interslice spacing of the output volume is specified by this
   option Output volume interpixel spacing is the same as the input
   volume). Default:1.

-d(x:y:z)
   This option selects the size and number of slices in the output
   volume. Default: 256:256:100.

-Dx(x:y:z)
   This option selects the orientation of the X axis in the output
   volume with respect to the input volume axis. Valid values for X,Y,Z
   are 0,1,-1 and two of the three must be 0. Default: 1:0:0 (original
   volume X axis).

-Dy(x:y:z)
   This option selects the orientation of the Y axis in the output
   volume with respect to the input volume axis. Valid values for X,Y,Z
   are 0,1,-1 and two of the three must be 0. Default: 0:1:0 (original
   volume Y axis).

-Dz(x:y:z)
   This option selects the orientation of the Z axis in the output
   volume with respect to the input volume axis. Valid values for X,Y,Z
   are 0,1,-1 and two of the three must be 0. Default: 0:0:1 (original
   volume Z axis).

SEE ALSO
========

tal_programs, tal_resize

NOTES
=====

Currently, no smoothing filters have been defined.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
