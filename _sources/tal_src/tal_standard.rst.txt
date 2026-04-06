===============
tal_standardize
===============


NAME
====

tal_standardize - Performs statistical table lookups and transforms on
volumes.

SYNOPSIS
========

tal_standardize [options] inputtemp outputtemp

DESCRIPTION
===========

This program reads and input volume, performs a lookup table based
transform and writes an output volume.

INPUTTEMP is the input volume template. OUTPUTTEMP is the volume
template for the computed volume. The program computes the values of a
function between two user specified values (at a given resolution).
These values are placed into a table. Each pixel value in the input
image is looked up on this table for an output value. Linear
interpolation is performed for input values which fall between table
values. Input values falling outside the table range are clamped to the
output values at the table extremes and the user is warned that this
clamping occurred.

Options:

-A(tabletype)
   This option allows the user to select the type of table operation
   they wish to perform. TABLETYPE can be one of the following:

t2z

This is the default table transform. It assumes the input pixel values
are from a T distribution with degrees of freedom given by -df1. The
output pixel value is the Z score with the same CDF value as the CDF
value of the T score.

t2p

This is the t to p transform. It assumes the input pixel values are from
a T distribution with degrees of freedom given by -df1. The output pixel
value is area under the PDF curve from the absolute value of the input
pixel to infinity. (One tailed cumulative probability above the absolute
value of the t score.)

z2p

This is the z to p transform. It assumes the input pixel values are from
a normal distribution. The output pixel value is area under the PDF
curve from the absolute value of the input pixel to infinity. (One
tailed cumulative probability above the absolute value of the z score.)

f2p

This is the f to p transform. It assumes the input pixel values are from
an f distribution with degrees of freedom given by -df1 and -df2. The
output pixel value is area under the PDF curve from the absolute value
of the input pixel to infinity. (One tailed cumulative probability above
the absolute value of the f score.)

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 4 bytes (floating point).

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

-c
   This specifies command line mode. The input and output templates are
   ignored. The user is prompted for an input value and the program
   outputs the input value followed by the table generated output value
   in tab form on standard out. This mode continues until an end of file
   is reached. This can be useful when standard statistic tables are not
   immediately available.

-tmin(min)
   This option allows the user to specify the minimum input value in the
   table. The default is:-10.0

-tmax(max)
   This option allows the user to specify the maximum input value in the
   table. The default is:10.0

-tstep(step)
   This option allows the user to specify the step size for input values
   in the table. The default is:0.01.

-o(offset)
   This allows a pixel value offset parameter to be specified. This
   value is subtracted from the pixel value before it is looked up from
   the table and added to the table output value. The default is:0.

-df1(df)
   This allows the user to specify a degrees of freedom parameter for
   statistical table (such as the Student's t) which require it. The
   default is:1.

-df2(df)
   This allows the user to specify a second degrees of freedom parameter
   for statistical table (such as Chi-square) which require it. The
   default is:1.

SEE ALSO
========

tal_programs, tal_math

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
