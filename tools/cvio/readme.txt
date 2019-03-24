---------------------------------------------------------------------------
idl_cvio                                                      Release 1.1.1
---------------------------------------------------------------------------

These file are, in general, Copyright (c), 1999-2005 Randall Frank

    See the file "COPYING" and the end of this document for further remarks.

Introduction:

	This archive contains the source to the University of Iowa, 
Hospitals and Clincs, Department of Neurology, CVIO library.  This
library is designed to allow for dynamic I/O operations on up to
3D data samples.  The API is inherently time aware and allows for
controlled access to the time varying data.  An IDL wrapper is
provided to allow a user to call the entire CVIO library from IDL.
Note: the shared memory interface is not supported under Windows.

		---------------------------------------

IDL interface to the CVIO library:

Variables ending in "_" are outputs.

err = CVIO_GET_ERROR_MESSAGE(errval,msg_)
err = CVIO_LISTSHMEM(names_,num_names_)
err = CVIO_CREATE(name,nsamples,datatype,ndims,dims,flags)
err = CVIO_DELETE(filename)
err = CVIO_OPEN(filename,access,stream_)
err = CVIO_CLOSE(stream)
err = CVIO_HAS_WRITER(stream,nwriters_)
err = CVIO_MAX_LENGTH(stream,length_)
err = CVIO_ADD_SAMPLES(stream,time,buffer,count)
err = CVIO_READ_SAMPLES(stream,time_[,buffer_],count) (time_ is input and output)
err = CVIO_GETATTRIBUTE(stream,attr,value_)
err = CVIO_SETATTRIBUTE(stream,attr[,value])  (no value removes the attr)
err = CVIO_GETATTRIBUTELIST(stream,names_,num_names_)
timestamp = CVIO_CURRENT_TIME()
err = CVIO_READ_NEXT_SAMPLES(stream,time_[,buffer_],count)
err = CVIO_TELL(stream,cur_sample_,cur_time_,num_samples_)
err = CVIO_SEEK(stream,is_time_flag,value)
err = CVIO_DATATYPE(stream,datatype_,num_dims_,dims_)

err=AIR_ALIGN(standard,reslice[,COST_FUNCTION=c][CONVERGENCE=c]
        [FIT_MODEL=f][INTERPOLATION=i][PARTITIONS=pp][THRESHOLD=t]
        [STEPS=sss])

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

1.0.0	23 Dec 00	Initial release, with  Windows support

1.1.0	28 May 01	Updated for time only access and signed shorts.

1.1.1	26 Mar 05	Added the socket support.

Acknowledgments:

	The author would like to thank the following people for their
input into this software.
	
	Tom Grabowski
	Chris Smyser
	Derek Foreman

Other contributors:

There are several files in this distribution from other authors which
are not covered by the notices in the file "COPYING":

	zlib - (C) 1995-1998 Jean-loup Gailly and Mark Adler (see the readme)

