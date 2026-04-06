=========
tal_merge
=========


NAME
====

tal_merge - Computes a simple linear combination between two volumes.

SYNOPSIS
========

tal_merge [options] inputtemp inputtemp2 outputtemp

DESCRIPTION
===========

This program computes an output volume which is a linear combination of
two input volumes.

INPUTTEMP is the first input volume template. INPUTTEMP2 is the second
input volume template. OUTPUTTEMP is the volume template for the
computed volume. The output volume is computed according to the
following formula:

OUTPUT = ((V1+OFFSET1)*MULT2) + ((V2+OFFSET2)*MULT2) + CONST

The default values for all the parameters will compute an output volume
which is the average of the input volumes (the sum of 0.5 times each of
the input volumes).

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

-m(mult1)
   This allows for the MULT1 parameter to be specified. The default
   is:0.5.

-n(mult2)
   This allows for the MULT2 parameter to be specified. The default
   is:0.5.

-o(offset1)
   This allows for the OFFSET1 parameter to be specified. The default
   is:-256.

-p(offset2)
   This allows for the OFFSET2 parameter to be specified. The default
   is:-256.

-c(const)
   This allows for the CONST parameter to be specified. The default
   is:256.

SEE ALSO
========

tal_programs, tal_math

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
