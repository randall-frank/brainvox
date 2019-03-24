---------------------------------------------------------------------------
idl_tools                                                     Release 1.5
---------------------------------------------------------------------------

These file are, in general, Copyright (c), 1999-2003 Randall Frank

    See the file "COPYING" and the end of this document for further remarks.

Introduction:

	This archive contains a number of tools I have written to extend
the functionality of Research Systems' IDL product. Complete source code 
is included for all the DLMs.  
	I just thought some of you might find these things useful, so I 
figured I would put the source put for people to play with.

This release consists of the following broad features:

	* Sockets API for Windows and Unix
	* An interface to gzip compression for files and variables
	* An async sound playing API for Windows (DirectSound based)
	* A polygon mesh decimation routine
	* A TWAIN interface for Windows
	* An array slabbing API
	* A generic interface for calling Windows DLL functions directly
	* A simple shared memory/memory mapped file interface

To use these APIs, drop the idl_tools.dll and idl_tools.dlm files into
your IDL directory under Windows or place the idl_tools.so and idl_tools.dlm
files into the location where you start up IDL under Unix.  You can 
check for the presence of the APIs using the HELP,/DLM command under
IDL.  The DLM was built using IDL 5.6 and may need changes for other
versions of IDL.
	
The individual APIs are documented as follows:

		---------------------------------------
Sockets:

socket = SOCK_CREATEPORT(portnum)
 
 	Creates a socket listening on the specified port for a new
 	connection.  SOCK_SELECT returns true for this socket if
 	there is an attempt to connect to it (which should be
 	serviced by SOCK_ACCEPT).

err = SOCK_CLOSE(socket)
 
 	Close and free the socket in question.

socket = SOCK_CONNECT(host,port[,BUFFER=num][,/NODELAY])
 
 	Connect to a socket listener on some specified host and port.
 	The returned socket can be used for I/O after the server
 	"accepts" the connection. The BUFFER keyword can be used to
	set the socket buffer size. For high-performance TCP/IP
	networks (e.g. gigE), higher bandwidth can be achived by
	setting the buffer size to several megabytes.  Setting
	the NODELAY keyword disables the Nagle algorithm for the
	socket (appropriate for applications with large numbers
	of small packets).

socket = SOCK_ACCEPT(socket[,BUFFER=num][,/NODELAY])
 
 	Accepts a requested connection and returns a socket on which
 	I/O can be performed. The BUFFER keyword can be used to
	set the socket buffer size. For high-performance TCP/IP
	networks (e.g. gigE), higher bandwidth can be achived by
	setting the buffer size to several megabytes.  Setting
	the NODELAY keyword disables the Nagle algorithm for the
	socket (appropriate for applications with large numbers
	of small packets).

nbytes = SOCK_SEND(socket, variable)
 
 	Sends the raw byte data from the IDL variable on the
 	socket.  Returns the number of bytes sent or -1 for error.
 	Note: no byteswapping is performed.

nbytes = SOCK_RECV(socket, variable [, MAXIMUM_BYTES=b])
 
 	Reads the raw data available on the socket and returns a BYTE
 	array in variable.  The maximum number of bytes to read
 	can be specified by the MAXIMUM_BYTES keyword.  The default
 	is to read all the data available on the socket.
 	Note: no byteswapping is performed.

err = SOCK_QUERY(socket [,AVAILABLE_BYTES=a] [,REMOTE_HOST=r]
 		[,IS_LISTENER=l])
 
 	Returns various information about the socket in question.
 		AVAILABLE_BYTES : number of bytes available for reading.
 		REMOTE_HOST: host number of the remote host the socket
 			is connected to.
 		IS_LISTENER: true if the socket was created using
 			SOCK_CREATEPORT()

err = SOCK_SENDVAR(socket, variable)
 
 	Sends a complete IDL variable to a socket for reading by
 	SOCK_RECVVAR.  The variable must be one of the basic types,
 	but strings and arrays are sent with array dimensions and
 	lengths intact.  
 	
	Note: This is the easiest way to send a complete variable from
 		one IDL to another.  The receiver will automatically
 		byteswap the data if necessary.  One should be careful
 		not to mix calls to SOCK_SEND/RECV and SOCK_SENDVAR/RECVVAR
 		as the latter send formatted information.  You can use
 		the two calls on the same socket as long as they are
 		paired.

err = SOCK_RECVVAR(socket, variable)
 
 	Reads an IDL variable from the socket in the form written by
 	SOCK_SENDVAR.  The complete variable is reconstructed.
 	See: SOCK_SENDVAR for more details.
 
out = SOCK_SELECT(sockets[], timeout)
 
 	Checks to see if there is data waiting to be read or a 
 	connection has been requested for a list of sockets.
 	The return value is -1 on error, scalar 0 if no sockets
 	are ready or returns a list of the sockets which are ready.
 	The routine waits the number of seconds specified by the
 	timeout argument for sockets to become ready.  A timeout
 	value of 0 results in a poll of the sockets.

host = SOCK_NAME2HOST(['name'])
 
 	Converts the ASCII host name into an unsigned long host value.
	If ['name'] is not specified, the local host name is used.

name = SOCK_HOST2NAME([host])
 
	Converts the unsigned long host value into an ASCII hostname.
	If [host] is not specified, the local hostname is returned.

  	General notes: 
 		All error codes return -1 on failure.
 		The socket identifier is NOT the actual underlying OS
 		socket number, it is an IDL abstraction.
 		ports are 16bit integers.
 		hosts are unsigned 32bit integers.

		---------------------------------------
Gzip:

	The zlib interface comes in two forms.  The G(UN)ZIP
	interface performs the same operation as the 'gzip' and
	'gunzip' utilities.  It will allow data to be compressed/
	decompressed between disk files and IDL variables. 
	The second interface is the (UN)PACKVAR interface.  This
	uses zlib to compress/decompress an IDL variable into a
	special memory format.  This format is compressed, but
	includes a header that describes the IDL varible and its
	endian in detail.  On deceompress, the original IDL
	varible is reconstructed fully, datatype, dimensions
	and the endian is properly converted.  It can be very
	useful as an "in memory" SAVE file and in conjunction
	with networking.

	In all cases, "err" is the number of bytes transferred or
	-1 for an error.

err = GZIP( 'infile', 'outfile')
err = GZIP( 'infile', varout)

	Compress the contents of the input file to the output file
	or into the contents of an IDL byte array in .gz format.  
	The outfile can be read with gunzip or the output array
	can be written to disk and read with gunzip.

err = GZIP( var, 'outfile')
err = GZIP( var, varout)

	Compress the binary contents of the IDL variable VAR to
	the output file or into the contents of an IDL byte array 
	in .gz format.  Note that the IDL format of the input IDL
	variable and its endian is lost in this conversion.

err = GUNZIP( 'infile', 'outfile')
err = GUNZIP( var, 'outfile')

	Uncompress the input (.gz) file or the compressed data in
	the variable VAR to the specified output file.  Reading the
	file 'infile' into memory and then calling the second
	function is essentially the same as the first function. 

err = GUNZIP('infile', outvar [,LENGTH=l[,OFFSET=o])
err = GUNZIP( var, outvar [,LENGTH=l[,OFFSET=o])

	Uncompress the input (.gz) file or IDL variable into an IDL 
	byte array.  The LENGTH and OFFSET keywords specify the number 
	of bytes to skip over in the (uncompressed) stream and the
	number of bytes to return in OUTVAR.

err = PACKVAR( invar, outvar)
err = UNPACKVAR( invar, outvar)

	These two routines pack an IDL variable into another IDL
	variable (byte array) using gzip compression.  Note: the
	unpack function will reconstruct the original variable 
	type and there is about a 50 byte overhead for the 
	compression.  So, you won't see any real compression unless
	the input variable is more than 100 bytes in length or so.

		---------------------------------------
Notes: this API uses DirectSound. You must have DirectSound installed
for the API to work.  In addition, the acquisition calls use DirectSound
5.0 routines and they will not work on versions of Windows NT prior to
5.0.  The code is dynamically loaded and will automatically detect the
presence of the appropriate levels of DirectSound.  In general, you can
use the playback API on NT4/95/98 and the acquisition code under 2k/95/98.

Sound:

handle = SND_PLAY(sounddata, rate)
 
 	Plays a sampled sounds stream at the given rate (samples per second).
 	Sounddata can be a 1D array (mono) or a [2,N] array (stereo).
 	The data can be in 8bit (unsigned) or 16bit (signed), all other
 	types are converted to 16bit signed internally.  The sound
 	plays asynchronously and can be controlled by the returned handle.
 	Once the sound has finished playing, the handle is no longer valid
 	(this is a one-shot system).  Note: multiple sounds can be played
 	simultaneously and all handles will be unique.

err = SND_STOP([handle])
 
 	If a handle is specified (and playing), its playing will
 	be stopped and its memory reclaimed.  If no handle is specified,
 	ALL playing sounds will be stopped and reclaimed.

err = SND_QUERY([handle])
 
	Returns 1 if the specified handle is still playing.
	Returns -1 if the specified handle does not exist (or is done playing).
	If no handle is specified, the function returns 1
	if any sounds are playing, otherwise it returns 0.

	General notes:
		The system is based on DirectSound.  It will not init
		if DirectSound is not installed.
		All error codes return -1 on failure.
		Sound handles are passively reclaimed whenever any of
		the SND_XXX routines are called.  This means that any
		sounds that play themselves out will not have their
		memory/resources reclaimed until the next SND_XXX call
		or IDL shutdown.

err = SND_ACQ_START(buffersize)
	
	Returns -1 on failure, 0 on success.
	BUFFERSIZE is the size of the acquire buffer in seconds.  
	Sound will start recording and be placed into the buffer.  
	The caller must call SND_ACQ_DATA() to keep the buffer empty.  
	BUFFERSIZE should be at least twice the size of the amount 
	of data one wishes to digitize.  Note: on somme platforms,
	the buffer is circular, so you should call SND_ACQ_DATA()
	at least one every 1/2 BUFFERSIZE to keep data from being
	lost.

err = SND_ACQ_STOP()

	Returns -1 on failure, 0 on success.
	This call stops the dynamic acquisition.

err = SND_ACQ_DATA(duration, data, rate)

	Returns -1 on failure, 0 on success.
	DURATION is an input argument that specifies the duration of
	the acquire in seconds.
	Data returns the actual data (byte or int form, one or 2 channels).
	Rate returns the sampling rate at which the sound was acquired (Hz).
	This function returns the next DURATION seconds of data from the
	cicular buffer.  If DURATION seconds of data are not available,
	the routine blocks until they become ready.

		---------------------------------------
Decimation:

ntris = MESH_THIN(verts,conn,numtris,vout,cout[,BLOCKSIZE=b][,PRESERVE_EDGES=p]
		[,AUXDATA_IN=in][,AUXDATA_OUT=out][,AUXDATA_WEIGHTS=w])

	Given a polygon mesh consisting of a [3,n] vertex array (VERTS) and
	a polygon connectivity array (CONN).  This routine will reduce the
	density of the original mesh to NUMTRIS triangles (if possible) by
	collapsing polygon edges.  The return value is the number of output
	triangles (generally the same as NUMTRIS) with VOUT and COUT being
	the output vertex and connectivity arrays.  Note that this routine
	will not generate any new vertices.  The verts returned in VOUT will
	be a subset of those passed in VERTS.  If the PRESERVE_EDGES keyword
	is set, polygons along "open" edges of a mesh will not be removed.
	The BLOCKSIZE keyword is a performance trade-off.  The default value
	is about 20% of NUMTRIS.  It controls the number of triangles
	removed before triangle cost functions are re-evaluated.  A value
	of 1 results in a higher quality reduction, but is VERY slow.
	AUXDATA_IN and AUXDATA_OUT allow the user to specify a vector of data
	which is attached to each vertex.  The righmost array dimensions of
	the input array must match the number of vertices input.  AUXDATA_WEIGHTS
	is a vector of weighting values (default values are 1.0).
	This function removes polygon edges with the lowest cost based on:

	edgelength*curvature + sum( auxweight(i)*|v0:auxvalue(i)-v1:auxvalue(i)| )
	
	where v0 and v1 are the verts on the edge to be collapsed and
	edgelength is the distance between them.  Curvature is a measure of
	the local maximum curvature (0.0=flat 1.0=max curvature).  For
	example, for N verts, the AUXDATA_IN could be color data [3,N]
	array of bytes.  In this case, AUXDATA_WEIGHTS might be 
	[1./255.,1./255.,1./255.] to normalize the local color differences.

	AUXDATA_OUT is the same as AUXDATA_IN with the data for removed
	vertices removed (also, the format will be float or byte).

		---------------------------------------
TWAIN:

IhaveTWAIN = TWAIN_PRESENT()

	Returns 1 if the machine as a TWAIN Data Manager installed and
	operational.  
	Returns 0 if the machine has no Data Manager.

error = TWAIN_SELECT()

	Implements the TWAIN "Select Source" menu item.  Allows the
	user to select from between the installed data sources via
	a modal dialog.

error = TWAIN_ACQUIRE(image,info[,r,g,b][,/HIDE_GUI])

	Performs an image acquisition.  The image will be in bytes and
	can be [x,y] (monochrome and pseudo color) or [3,x,y] (Truecolor).
	Various information about the acquired image is returned in the
	INFO struct (similar to the QUERY_XXX routines with DPI added).
	The R,G,B optional arguments return the color palette to be used
	with the image (if any).  If a palette is returned with a TrueColor
	image, it is the optimal 8bit palette for the image to be dithered
	into.

		---------------------------------------

SLAB,varin,varout[,POSITION=p][,SIZE=s][,STRIDE=str][,/CLIP]
	[,/PASTE_ARRAY[,PASTE_POSITION=pp][,PASTE_STRIDE=ps]]

	This API copies a block of data from VARIN to VAROUT.  The source
	block is defined to start at some position P, of size S samples,
	using the stride STR to walk through the source.  By default, VAROUT
	is created to hold this data exactly.  If PASTE_ARRAY is specified
	and VAROUT is an array of the same dimensionality and type as VARIN,
	the block will be pasted into the existing VAROUT array and the
	PASTE_POSITION and PASTE_STRIDE keywords can be used to specify
	any location and stride in the output array.  By default, if the
	regions hang outside of either of the arrays, an error will be
	thrown.  If /CLIP is specified, the regions will be clipped to
	the bounds of VARIN (and if PASTE_ARRAY is set, VAROUT as well).
	The position keywords default to a vector of zeros, the stride
	keywords to a vector of ones and the size keyword defaults to
	the extent of VARIN beyond the specified position.  Note: negative
	stride values are legal.

		---------------------------------------
EXTPROC:
	
	This interface allows an IDL program to directly access the
	exported functions in any Windows DLL.  This interface can
	replace the CALL_EXTERNAL wrappers for many functions under
	Windows.  The interface consists of two functions.  One to
	define the external function to IDL and one to query the
	external function table.  Note: this is a very powerful and
	potentially dangerous function.  Misuse can cause your
	computer to crash.  Users are warned to take extra caution
	when using these routines.

err = EXTPROC_DEFINE(idlname,dllfilename,dllfuncname,funcsig[,/CDECL])

	This function takes four string arguments and will define
	an IDL procedure or function to map to the function 
	DLLFUNCNAME in the Windows DLL named DLLFILENAME.  The IDL
	function or procedure will be named IDLNAME.  The function
	is assumed to have parameters specified by the string FUNCSIG.
	If the DLL function follows the CDECL calling conventions,
	the /CDECL flag must be set.  The function signature (FUNCSIG)
	is a string of the following form:

	"[ret]([arg[,arg[,arg...]]])"

	Where [ret] and [arg] are single characters.  [ret] can be:

    v : void - use this to select no return value
	p : pointer - see below
	c : byte
	i : short
	I : unsigned short
	l : long int
	L : unsigned long int
	f : float
	d : double

	[arg] can be anything except 'v'.  Each letter defines one 
	of the arguments to the external function.  For the letters
	c,i,I,l,L,f,d, IDL will convert the input IDL variable to
	a scalar of the appropriate type and pass the result to the
	DLL function (if the function signature is incorrect, it is
	likely that IDL will crash).  'v' is only available before
	the first '(' and signifies that there is no return value 
	from the function.  If a return value is specified, an
	IDL function will be defined which return a scalar of the
	type specified by [ret].  Pointers ('p') follow slightly
	different rules.  If the return value is a pointer, the
	memory address returned by the function will be placed in
	an IDL unsigned long and returned to the IDL application.
	If an argument is specified as a pointer and the IDL variable
	passed is a scalar, then the value of that IDL variable will
	be passed to the function as the pointer.  If an argument
	has a pointer signature and the IDL variable passed is an
	array, the address of the start of the memory storage in 
	the IDL array is passed as a pointer to the DLL function.
	Thus, if a function takes an argument of (float *) and the
	user knows the pointer needs to point to 20 floats, an
	IDL float array of length 20 can be passed directly as
	the pointer argement.  If an IDL string is passed as a
	pointer argument, a pointer to the string value is passed
	to the function.  No type casting of pointer arguments
	is performed.

	Note: there can be an issue when attempting to compile code
	that relies on functions defined by EXTPROC_DEFINE, before
	the actual EXTPROC_DEFINE call has been made.  IDL will not
	be aware of the routine and may have problems compiling. One
	way to work around this is to use CALL_FUNCTION and CALL_PROCEDURE
	to call the actual functions defined by EXTPROC_DEFINE.  This
	situation is similar to that which can arise with LINKIMAGE.

	Examples:
		The function "_cdecl double sin(double)" in msvcrt.dll.

		print,EXTPROC_DEFINE("mysin","msvcrt.dll","sin","d(d)",/CDECL)

		This defines the function MYSIN() to IDL to take a single
		argument (as a double) and to return a double.

		The function "bool MessageBeep(int)" in user32.dll.

		print,EXTPROC_DEFINE("mymessagebeep","user32.dll", $
			"MessageBeep","i(i)")

		Allows one to use: print,MYMESSAGEBEEP(x) in IDL.

status = EXTPROC_QUERY(idlname)

	The function returns a null string if the named function has not
	been defined previously or the function signature if it has 
	already been defined.

var = EXTPROC_DEREF(addr[,/STRING][,INT=][,LONG=][,UINT=][,ULONG=]
		[,FLOAT=][,DOUBLE=][,BYTE=])

	This function "dereferences" a pointer and copies the data it
	points to into an IDL variable.  The address in memory is 
	specified by the variable ADDR.  By default, a single byte of
	data is copied into an IDL byte array (the same as the keyword
	BYTE=1).  One of the keywords should be used to select the type
	of IDL array to return and the number of elements of that type
	to return is the value of the keyword.  For example, specifing
	LONG=8 will cause 8 4byte longs to be read starting at the memory
	address specified by ADDR and their values will be returned as
	an 8 element IDL long array.  If /STRING is specified, this 
	function will return an IDL string that is a copy of the C
	(null terminated) string located at ADDR.

	Note: caution should be taken with this function as improper use
	can crash IDL.

		---------------------------------------

Shared memory/memory mapped files:

	This set of functions are not yet complete, but are basically
	functional already.  Essentially, at this point, it is an
	interface to memory mapped files.  The pure shared memory
	funcations are not yet finished.  See idl_sharr.c for details.

var = SHARR_MAP("filename",type,dim[,/CREATE][,/READONLY][,OFFSET=off])
status = SHARR_TEST(var)
error = SHARR_FLUSH(var)

TODO:
name = SHARR_ALLOC_MEM(ID,type,dim)
error = SHARR_FREE_MEM(name)

		---------------------------------------

Updates/bugs:

I (Randall Frank) am the only person working on this software.  If you
have a question or a problem, you can email me, but I do not know how much
support I can give you.  If you find any bugs, please let me know and I
will try to address them.  Also, I am open to suggestions for enhancements
or additional  programs.

Randall Frank
randall-frank@computer.org

Known issues/bugs:

	* The TWAIN interface should use memory transfer instead of bitmaps

Future possibilities:

	* HW accelerated volume rendering
	* Automated Image Registration
	* Image processing functions
	* OG keyframe animation editor
	* Wavefront OBJ file I/O
	* FFTW interface

Revision history:

1.0.0	22 Apr 99	Initial release
1.0.1	30 May 99	Fixed a couple of mesh bugs and added SLAB
1.0.2	18 Sep 99	Made the sound APIs dynamically loaded to 
					improve Windows NT support
1.1.0	26 Sep 99	Fixed an issue with older DirectSound & NT
					Added the shared memory API
1.2.0	12 Mar 00	Fixed a bug in the sockets code for handling
					of a keyword.  Added code necessary to support
					DirectX 7 with the sound playing API.  Minor
					tweaks to the decimation routine.  Added the DLL
					direct access functions (EXTPROC).
1.3.0	10 Jun 00	Added a shared memory interface.
1.3.1	29 May 01	Added TWAIN GUI hide flag.
1.5.0	12 Jun 02	Added gzip memory to memory case, added support
                    for high-performance sockets (e.g. gigE) and fixed
					issues stemming from changes in the IDL and  
					DirectX/DirectSound APIs.

Acknowledgments:

	The author would like to thank the following people for their
input into this software.
	
	Stan Melax - cost functions for mesh decimation
	The TWAIN Working Group - http://www.twain.org - example TWAIN source

Other contributors:

There are several files in this distribution from other authors which
are not covered by the notices in the file "COPYING":

	zlib - (C) 1995-1998 Jean-loup Gailly and Mark Adler (see the readme)

	DynaCall() - Ton Plooy, Windows Developer's Journal, Aug 1998.
