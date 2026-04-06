=======
tal_edm
=======


NAME
====

tal_edm - Computes a Euclidean distance map for an input volume.

SYNOPSIS
========

tal_edm [options] inputtemp outputtemp

DESCRIPTION
===========

This computes a Euclidean distance map (EDM) for objects in the input
volume. The value of a voxel in an EDM is equal to the distance from
that voxel to the edge of an object in the volume. A thresholded EDM can
be used to quickly erode a volume. A thresholded EDM of the inverse of
an input volume can be used to quickly dilate a volume. The objects
within a volume are selected by simple thresholding. Voxels greater than
the threshold are considered to be part of the objects. The operation
can be performed in 3D, or as a series of 2D planar operations.

INPUTTEMP is the input volume template. OUTPUTTEMP is the output volume
template. The output volume is an 8bit volume unless the -s option is
specified, in which case it is 16bit.

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 2 bytes (unsigned shorts).

-o(offset)
   16 bit data offset. All pixels have this offset value added before
   they is used. The default is 0.

-f(start)
   This specifies the slice number for the first image in the volume.
   Default:1.

-l(end)
   This specifies the slice number for the last image in the volume.
   Default:119.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using.

-t(thres)
   This specifies a threshold value. Voxels above this value are
   considered to be part of the object. If the threshold value is
   negative, voxels which are below the negative of the threshold value
   will be considered part of the object (allowing for inverse
   thresholding). The default value:100

-2d
   This option specifies that the program is to perform plane by plane a
   2d version of the operation. The default:full 3d operation.

-h(histfile)
   This option specifies that a histogram of the EDM volume is to be
   written to the specified file. By default, histograms are not saved.

-s(scale)
   This option specifies an integer scale factor for the EDM volume. The
   value should be between 1 and 256. The output volume will be 16bits
   if this option is used and will have some additional sub-voxel
   accuracy that is not found in the unscaled EDM. The voxel value in
   the scaled EDM should be divided by the scale factor to get the true
   (fractional) EDM value. The default:8bit unscaled EDMs.

-sip(interpixel)
   This option allows the user to specify the interpixel spacing. This
   can be used in conjunction with the scaled (-s) option to improve the
   accuracy of 3D EDMs. The default:1.0.

-sis(interslice)
   This option allows the user to specify the interslice spacing. This
   can be used in conjunction with the scaled (-s) option to improve the
   accuracy of 3D EDMs. The default:1.0.

SEE ALSO
========

tal_programs

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
