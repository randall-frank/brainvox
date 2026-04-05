=======
tal_CDA
=======


NAME
====

tal_CDA - performs change distribution analysis on image volumes.

SYNOPSIS
========

tal_CDA [options] inputtemp outputfile

DESCRIPTION
===========

This program reads the volume specified by the input template and
performs change distribution analysis. It first detects local maxs and
mins based on the neighborhood selected by the '-r' option and the step
sizes. It then checks for kurtosis in the resulting distribution
(gamma2) and reports the points whose are significant by the gamma1
test. The output includes the location of each local max and min
weighted by their neighborhood, the z scores and Talairach space
location of the points. The output is sent to the specified file or to
stdout if "-" is specified as an output file.

Options:

-x(dx)
   The x axis size of the image in pixels. The default is 256 pixels.

-y(dy)
   The y axis size of the image in pixels. The default is 256 pixels.

-z(dz)
   The image depth in bytes. The default is 2 bytes (unsigned shorts).

-s(ddx)
   The stepping size in the x axis. The default is 2 pixels.

-t(ddy)
   The stepping size in the y axis. The default is 2 pixels.

-u(ddz)
   The stepping size in the z axis. The default is 2 pixels.

-v
   The entire volume of data is to be analyzed. The default is to
   restrict analysis to Talairach space.

-g
   Uses an iterative computation of gamma1 and gamma2 which removes one
   local max/min for each pass until significance of the gamma
   statistics is lowered.

-verbose
   Reports to stderr information regarding each pass when using the -g
   option.

-o(offset)
   16 bit data offset. All 16 bit data have this offset value added
   before the data is used. The default is -256.

-d(stddev)
   This selects the z score value above which points are considered
   significant. The default is 2.575.

-ties(max_ties)
   Due to smoothing, a local max/min value may occur multiple times in
   the local search area. Up to this proportion of multiple values are
   allowed before the point is rejected as a local min/max. The default
   is 0.25.

-n(num)
   Number of patient volumes summed to make the input volume. The
   default is 1.

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

-r(dx[:dy:dz])
   This specifies the radius (in terms of number of stepping sizes in
   each dimension) within which a point must be a local min/max to be
   considered. If only DX is specified, it is assumed that DY and DZ are
   the same as DX. Individual values may be specified for all axis by
   separating the numbers by colons (:). The default is: 2:2:2.

-thres(V)
   In this mode, the gamma statistics are ignored, and all points with z
   scores (absolute values) over V are output.

-m(masktemp)
   This option allows the user to specify an 8bit mask volume. Only
   voxels for which the entire sphere of the search area (-r) is within
   the mask are considered as possible mins or maxs.

SEE ALSO
========

Brainvox Program Reference Guide, tal_programs

NOTES
=====

Talairach space assumes the same conventions as the Brainvox program.

This program is based on information in the following paper: Fox PT,
Mintun MA, Reiman EM, Raichle ME; Enhanced Detection of Focal Brain
Responses Using Intersubject Averaging and Change-Distribution Analysis
of Subtracted PET Images. J. Cereb Blood Flow Metab 8:642-653 (1988).

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
