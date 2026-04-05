======
tal_ls
======


NAME
====

tal_ls - lists the status of detected network volume servers.

SYNOPSIS
========

tal_ls [options]

DESCRIPTION
===========

This program can be used to display the status of any network/shared
memory volume server. Currently, Brainvox, Voxblast and
tal_mkshm/tal_rmshm are the only supported servers. The program will
collect and display the size and nature of the volumes that each copy of
Brainvox running on the current machine has loaded and will display the
same information for any default Voxblast serevrs as well. For shared
memory volume created using tal_mkshm, the volume template and the size
of the shared memory volume in bytes is displayed. The user may restrict
the search (asking for info on a specific server) via command line
options.

Options:

-n(n)
   Search a single invocation of Brainvox. N is the order in which the
   Brainvox copies were launched, starting at 0. The default is to
   search all running invocations.

-h(host)
   This selects a specific hostname (TCP/IP) to search for a Voxblast
   server. The default is:localhost.

-p(port)
   This selects a specific port (TCP/IP) to search for a Voxblast
   server. The default is:-1.

-s(volnum)
   This selects a specific shared memory volume number to check the size
   of. The default is to search for all shared memory volumes.

SEE ALSO
========

Brainvox Program Reference Guide, tal_programs, tal_rmshm, tal_mkshm

NOTES
=====

ORIGIN
======

Brainvox, Human Neuroanatomy and Neuroimaging Lab, Department of
Neurology, University of Iowa
