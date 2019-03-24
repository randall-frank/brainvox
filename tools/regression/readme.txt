---------------------------------------------------------------------------
cvio_reg                                                      Release 1.1.0
---------------------------------------------------------------------------

These file are, in general, Copyright (c), 1999-2000 Randall Frank

    See the file "COPYING" and the end of this document for further remarks.

Introduction:

	This archive contains a DLM implementing an incremental regression
system with support for various test statistics.  The algorithms used
are based on the functions provided by Applied Statistics routine 274
with some additions.  The IDL wrapper includes a parallel interface,
making it possible to run hundreds if not thousands of regressions
in parallel.

	This DLM is part of the University of Iowa, Hospitals and Clincs,
Department of Neurology, CVIO library.  

		---------------------------------------

r=REG_CREATE(vec,nvalues[,mask])
err=REG_DESTROY(r)
err=REG_REMOVE_OBS(r,y,obs[,w])
err=REG_ADD_OBS(r,y,obs[,w])
err=REG_CALC_SS(r[,mask],partial_,sst_,sse_,ssvar_)
err=REG_CALC_COEF(r[,mask],coef_[,STD_ERROR=e_])

		---------------------------------------

Updates/bugs:

I (Randall Frank) am the only person working on this software.  If you
have a question or a problem, you can email me, but I do not know how much
support I can give you.  If you find any bugs, please let me know and I
will try to address them.  Also, I am open to suggestions for enhancements
or additional  programs.

Randall Frank
randall-frank@acm.org

Known issues/bugs:

Future possibilities:

Revision history:

1.0.0	19 Jun 00	Initial release
1.1.0	8 Oct 00	Added (optional) weights for the observations

Acknowledgments:

	The author would like to thank the following people for their
input into this software.
	
	Tom Grabowski
	Chris Smyser

Other contributors:

There are several files in this distribution from other authors which
are not covered by the notices in the file "COPYING":

	zlib - (C) 1995-1998 Jean-loup Gailly and Mark Adler (see the readme)
	as274.[cf] - Morven Gentleman's AS75 routines by Alan Miller 
					(Applied Statistics 41/1992).
