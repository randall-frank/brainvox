===========
tal_optimal
===========


NAME
====

tal_optimal - Performs 3D optimal surface detection in cost function
volumes.

SYNOPSIS
========

tal_optimal [options] inputtemp outputimg

DESCRIPTION
===========

This program uses multi-dimensional dynamic programming to determine the
optimal 2 1/2 dimensional surface through a volumetric cost function.
The input is a cost function c=f(x,y,z) and the output is an image
z=g(x,y). The sum of f(x,y,g(x,y)) is minimized subject to the
constraints that \|g(x,y)-g(x+1,y)\| <= conn[x] and \|g(x,y)-g(x,y+1)\|
<= conn[y]. The algorithm is very fast and (provided that a temporary
accumulator does not overflow) exact. Range sets the connectivity
(surface smoothness) rules and can be set by the user. In addition, one
axis may be specified as a "wrap-around" axis. If such an axis is
selected, the solution at either end of the axis must also satisfy the
connectivity constraints.

INPUTTEMP is the input volume template. The input volume is a floating
point volume. OUTPUTIMG is the output image filename. The output image
is in 16bit format.

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-f(start)
   This specifies the slice number for the first image in the volume.
   Default:1.

-l(end)
   This specifies the slice number for the last image in the volume.
   Default:20.

-i(step)
   This specifies the increment number to go from one slice to the next.
   Default:1.

-b
   This specifies that 16bit images should be byte swapped before using.

-c(x:y)
   This option specifies the connectivity rules to use in the x and y
   axis. Default:1:1.

-w(wrap)
   This option selects the x or y axis (0 or 1) to be considered
   circular. The default:no wrapping.

-v
   This option enables verbose mode. Default: silent.

SEE ALSO
========

tal_programs

NOTES
=====

This program is based on an algorithm described in:

Frank, RJ, "Optimal Surface Detection Using Multi-dimensional Graph
Search: Applications to Intravascular Ultrasound", Master's Thesis,
University of Iowa, May 1996.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
