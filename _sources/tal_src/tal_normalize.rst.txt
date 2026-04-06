=============
tal_normalize
=============


NAME
====

tal_normalize - Normalizes a volume so that its mean voxel is a
particular value.

SYNOPSIS
========

tal_normalize [options] inputtemp masktemp outputtemp

DESCRIPTION
===========

This program reads an input volume and computes its mean value. An
output volume is then computed with the voxel values adjusted so that
the volume mean is set to a selected value. A mask volume is supported.
The input volume mean only includes those voxels for which the
corresponding voxel in the mask volume is non-zero. The equation used
is:

output = (input+offset)*((normal_value)/(volume_mean)) + const

INPUTTEMP is the input volume template. MASKTEMP is the mask volume
template. Use '-' for all pixels (no mask volume). OUTPUTTEMP is the
volume template for the output normalized volume.

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

-o(offset)
   16 bit data offset. All 16 bit data have this offset value added
   before the data is used. The default is: 0.

-b
   This specifies that 16bit images should be byte swapped before using.

-n(norm)
   This specifies the normalized average value for the output volume
   (The average of the masked pixels in the output volume will be equal
   to this value). The default is: 50.0.

-c(const)
   This specifies an offset value added to output volume pixels. The
   default is: 256.

-m
   If this option is specified, the output voxels outside the mask are
   set to 0. The default is to perform the same normalization as voxels
   inside the mask.

SEE ALSO
========

tal_programs

NOTES
=====

Currently, the mask volume must be an 8bit volume.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
