==========
tal_sphere
==========


NAME
====

tal_sphere - Computes the sphericity of a volume.

SYNOPSIS
========

tal_sphere [options] inputtemp

DESCRIPTION
===========

This program loads a volume into memory and computes the sphericity of
the volume. Sphericity is a measure of volume smoothness and is output
in pixel units. An 8bit mask volume may be specified using the options
and the output sphericity can be volumetric, or a average of the planar
(2d) shericity of all the volume slices.

INPUTTEMP is the input volume template.

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
   Default:119.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using.

-m(masktemp)
   Specifies a filename template for an 8bit mask volume.

-p
   Option to output the average planar sphericity. The default:3D
   sphericity.

SEE ALSO
========

tal_programs

NOTES
=====

This program is based on information in the following paper:

Friston KJ, Frith CD, Liddle PF,Frackowiak RSJ; Comparing Functional
(PET) Image: The Assessment of Significant Change. J. Cereb Blood Flow
Metab 11:690-699 (1991).

Only 8bit mask volumes are currently supported.

There are some differences between this implementation and the SPM
ks31.m computation. These include:

1) Volumetric basis or averaged slice based.

2) Zero valued derivative values allowed.

3) Uses one less change value per 3D scanline than ks31.m.

Generally, these differences are insignificant if the number of voxels
is large.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
