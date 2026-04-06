============
tal_resample
============


NAME
====

tal_resample - Resamples a volume into Talairach space.

SYNOPSIS
========

tal_resample [options] inputtemp outputtemp

DESCRIPTION
===========

This program reads an input volume and a Brainvox Talairach space
calibration file. It then resamples the input volume and outputs a
volume in Talairach space. A pre-interpolated nearest neighbor algorithm
is used for resampling. Output voxels are only computed for the voxels
considered to be located inside Talairach space. This leaves a large
border around a volume of 256x256 images.

INPUTTEMP is the input volume template. OUTPUTTEMP is the volume
template for the output Talairach space resampled volume.

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
   Default:124.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using.

-p(interpixel)
   This specifies the interpixel spacing in mm. The default:1.0.

-s(interslice)
   This specifies the interslice spacing in mm. The default:6.64.

-o(dz)
   This specifies the output interslice spacing in mm. The default:1.0

-L(minz:maxz)
   Set the minimum and maximum Z slices to compute for in talairach
   space. The default values are: -44.0:74.5

-d(value)
   This specifies a value for voxels outsize the resampled area. The
   default:256.

-t(talfile)
   This specifies the name of a Brainvox Talairach space calibration
   file to be used to direct the resampling. The default
   filename:\_talairach.

SEE ALSO
========

tal_programs

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
