===========
tal_cutting
===========


NAME
====

tal_cutting - Using a set of ROIs, "paints" portions of a volume.

SYNOPSIS
========

tal_cutting [options] roitemp inputtemp outputtemp

DESCRIPTION
===========

This program reads matched pairs of ROIs and images. The pixels outside
(optionally inside) the ROI are "painted" (set) to a user supplied
value. Generally the ROIs come from Brainvox. The default behavior is to
set pixels outside the ROIs to 0, thus cutting out a region from the
volume. For image slices where ROIs do not exist, all pixels in the
slice are considered outside the ROI.

ROITEMP is the ROI template. INPUTTEMP is the input volume template.
OUTPUTTEMP is the output volume template.

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 2 bytes (unsigned shorts).

-f(start)
   This specifies the slice number for the first image in the volume.
   Default:1.

-l(end)
   This specifies the slice number for the last image in the volume.
   Default:15.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using.

-noflip
   Specifies that ROIs are not to be flipped over the X axis before
   applying them. The default behavior is to flip them, as this is the
   behavior of Brainvox.

-v(value)
   This specifies the pixel value to paint with. The default is 0.

-s(scale) 
   This allows the ROI scaling factor to be changed. The default value
   is 0.5 which is appropriate for Brainvox. (In Brainvox, ROIs are
   traced on a 512x512 grid while images are generally 256x256.)

-o
   Paint the pixels inside of the ROI to the specified vlue. The default
   is to paint the pixels outside of the ROI, thus "cutting" the pixels
   in the ROI from the volume.

SEE ALSO
========

Brainvox Program Reference Guide, tal_programs, tal_math

NOTES
=====

The functionality of this program has been generally superceeded by
tal_math, however, this program is much faster than tal_math for its
more specific use.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
