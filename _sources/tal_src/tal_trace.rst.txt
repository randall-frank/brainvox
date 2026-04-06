=========
tal_trace
=========


NAME
====

tal_trace - Detects the closed contours in a volume on a slice by slice
basis.

SYNOPSIS
========

tal_trace [options] inputtemp roibasename

DESCRIPTION
===========

This program detects the contours on each slice of the input volume. The
contouring value may be slected by the user. Only exterior contours are
detected.

This program can be used to convert a voxel volume into an ROI volume.

INPUTTEMP is the input volume template. The input volume can only be in
8 or 16 bit format. ROIBASENAME is the base name for the output rois.
This name should include one template field. The program will fill in
the template field with the slice number and will append a string in the
form '\_000.roi' to the resulting basename. The appended string will
increment for each roi found in the slice. For example, if the
ROIBASENAME is 'base\_%%%' and on slice 10 there were 3 rois, the rois
for slice 10 would be named: 'base_010_000.roi',

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 1.

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
   This specifies a threshold value. The isocontour lines are generated
   at this pixel value. The default value:1

-omax
   Specifies that pixels outside of the image are to be considered to be
   at maximum value. By default, pixels outside the image are considered
   to be minimum valued.

-noflip
   Specifies that ROIs are not to be flipped over the X axis before
   applying them. The default behavior is to flip them, as this is the
   behavior of Brainvox.

-s(scale) 
   This allows the ROI scaling factor to be changed. The default value
   is 2.0 which is appropriate for Brainvox. (In Brainvox, ROIs are
   traced on a 512x512 grid while images are generally 256x256.)

SEE ALSO
========

tal_programs

NOTES
=====

Only 1000 ROIs are allowed on each slice and floating point input
volumes are not supported.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
