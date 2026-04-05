===========
tal_spatial
===========


NAME
====

tal_spatial - Filter program for the significance of tal_cluster output.

SYNOPSIS
========

tal_spatial size FWHM dimension

DESCRIPTION
===========

This program reads on standard input a list of clusters in the output
format of tal_cluster (IE x,y,z,avg value,size,serial number,threshold).
For each input cluster it computes the significance of the cluster
taking its spatial extent into account. The output of this program is a
new tab deliminated list with an eighth column added. This new column is
the probability the cluster appeared by chance. It is assumed that the
volume that was clustered was a Z map (SPM{z}).

SIZE is the size of the search volume in voxels.

FWHM is the smoothness of the search volume.

DIMENSION is the order of the search volume (1=1D,2=2D,3=3D).

SEE ALSO
========

tal_cluster tal_programs

NOTES
=====

This program is based on information in the following paper:

Friston KJ, Worsley KJ, Frackowiak RSJ, Mazziotta JC, Evans AC;
Assessing the Significance of Focal Activations Using Their Spatial
Extent. Human Brain Mapping 1:210-220 (1994).

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
