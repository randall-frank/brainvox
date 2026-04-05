========
tal_vbin
========


NAME
====

tal_vbin - Performs basic image file viewing operations.

SYNOPSIS
========

tal_vbin [options] binfile [xsize ysize [palettefile]]

DESCRIPTION
===========

This program reads a binary file and views it using an GLUT/OpenGL based
viewer. The program can guess the image size, read compressed files, and
apply a palette to the image. Support for 8, 16 and floating point
formats is included. The palette file is a 768 byte file in the form:
[RRR... GGG... BBB...]. BINFILE is the input binary filename. All GLUT
command line parameters are allowed as well. The display can be resized,
and the image will be stretched to fit. The display can also by quit by
pressing the

uncompressed file size in bytes, removing the header, dividing by the
size of a pixel in bytes, and taking the square root, assuming the image
to be square.

Note: unlike the rest of the tal_programs, options and their values are
space separated in this program.

Options:

-pid
   By default, the program will remain connected to the cmdline. With
   this option the program is disassociated from the command line and
   the process id of the display invoked is printed on stdout.

-head h
   This specifies the number of (uncompressed) bytes to skip before
   treating the rest of the file as an image. Default:0.

-pos x y
   This requests that the display be placed at the location x,y on the
   screen. Default: none.

-z1\|-8
   This specifies that the file is to be interpreted as unsigned byte
   pixels. Default: this is the default.

-z2\|-16
   This specifies that the file is to be interpreted as unsigned short
   pixels. By default pixels in this mode are automatically scaled
   (min,max) to (0,255) for display.

-z3\|-24
   This specifies that the file is to be interpreted as an RGB image
   with unsigned char components (RGBRGBRGB...).

-z4\|-float
   This specifies that the file is to be interpreted as IEEE floating
   point pixels. By default pixels in this mode are automatically scaled
   (min,max) to (0,255) for display.

-max max
   This specifies the maximum pixel value for 16bit and float scaling.
   Default: the largest pixel value in the image.

-min min
   This specifies the minimum pixel value for 16bit and float scaling.
   Default: the smallest pixel value in the image.

-swap
   This forces byte swapping for 16bit and floating images. Byte
   swapping is only supported for 16bit images. By default, the program
   attempts to guess the proper byte swapping.

-noswap
   This disables byte swapping for 16bit and floating images. Byte
   swapping is only supported for 16bit images. By default, the program
   attempts to guess the proper byte swapping.

-noflip
   This disables vertical image flipping. The default is to flip the
   image vertically.

-pal palfile
   This specifies the name of a palette file to be used to colorize the
   image. It is not used in -z3 mode.

-wait
   This specifies the program should run the the foreground and not
   spawn off an independant child process. It "waits" for the program to
   be exited.

SEE ALSO
========

tal_programs

NOTES
=====

Byte swapping is only supported for 16bit images.

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
