C
C	Include file for all FORTRAN programs within the brainvox suite
C
C	first is the common block vt which contains all the patient info
C	this block is read from the passed shared memory ID (shm_data)
C
	Integer*4	max_rgns
	Integer*4	max_images
	Parameter	(max_rgns = 60)
	Parameter	(max_images = 256)

	Character*200	imagename
	Character*200	patientinfo(3)
	Character*40	rgnnames(max_rgns)

	Integer*4	realimages
	Integer*4	imagesize
	Integer*4	interpimages
	Integer*4	numrgns
	Integer*4	rgntypes(max_rgns)
	Integer*4	volumedepth
	Integer*4	volume_zswap
	Integer*4	volume_bswap

	Real*4		volume_lims(2)
	Real*4		interpixel
	Real*4		interslice
	Real*4		squeezefactor
	Integer*4	cut_dir
	Integer*4	brainvox_options
C
C	options:
C
	Integer*4	OPTS_NONE
	Integer*4	OPTS_TESTING
	Integer*4	OPTS_ADVANCED
	Integer*4	OPTS_RJF
	Parameter	(OPTS_NONE	= 0)
	Parameter	(OPTS_TESTING	= 1)
	Parameter	(OPTS_ADVANCED	= 2)
	Parameter	(OPTS_RJF	= 4)

	common	/vt/realimages,imagesize,interpimages,numrgns,rgntypes,
     +		imagename,patientinfo,rgnnames,volumedepth,
     +		interpixel,interslice,volume_zswap,volume_bswap,
     +		squeezefactor,cut_dir,brainvox_options,volume_lims
C
C	block of shared memory id numbers
C
C	shared memory segment IDs
C
	Integer*4	shm_ipc
	Integer*4	shm_data
	Integer*4	shm_images
	Integer*4	shm_pts
	Integer*4	shm_texture
C
C	memory address of mount points (0 if segment is not attached)
C
	Integer*4	ipc_addr
	Integer*4	data_addr
	Integer*4	images_addr
	Integer*4	pts_addr
	Integer*4	texture_addr
	Integer*4	lut_addr
C
	Integer*4	shmem_owner  ! 0=client 1=owner
C
	common 	/shared_mem/shm_ipc,shm_data,shm_pts,shm_images,
     +		shm_texture,shmem_owner,
     +		texture_addr,ipc_addr,data_addr,images_addr,pts_addr,
     +		lut_addr
C
C	shared mem allocation sizes
C
	Integer*4	DATA_SHM_SIZE
	Integer*4	LUT_SHM_SIZE
	Integer*4	LUT_SHM_OFFSET   ! the actual data_shm_size
	Integer*4	LUT_IMG_OFFSET
	Integer*4	LUT_VXL_OFFSET
	Integer*4	LUT_TEX_OFFSET
	Integer*4	LUT_FN1_OFFSET
	Integer*4	LUT_FN2_OFFSET
	Integer*4	PTS_SHM_SIZE
	Integer*4	IPC_SHM_SIZE
	Integer*4	IMAGES_SHM_SIZE	  ! computed on the fly
	parameter	(LUT_SHM_SIZE = 3*256*4 + 2*256*256*4)
	parameter	(LUT_SHM_OFFSET = 4096)
	parameter	(DATA_SHM_SIZE = LUT_SHM_OFFSET+LUT_SHM_SIZE)
	parameter	(LUT_IMG_OFFSET = 0)
	parameter	(LUT_VXL_OFFSET = 256*4)
	parameter	(LUT_TEX_OFFSET = 2*256*4)
	parameter	(LUT_FN1_OFFSET = 3*256*4)
	parameter	(LUT_FN2_OFFSET = 3*256*4+256*256*4)
	parameter	(PTS_SHM_SIZE = (4097*4*4))
	parameter	(IPC_SHM_SIZE = 2048)
C
C	IPC declares
C
	Integer*4	IPC_READ
	Integer*4	IPC_WRITE
	Parameter	(IPC_READ  = 0)
	Parameter	(IPC_WRITE = 1)
	Integer*4	IPC_QUIT
	Integer*4	IPC_OK
	Integer*4	IPC_POINT
	Integer*4	IPC_CLEAR
	Integer*4	IPC_VLUTS
	Integer*4	IPC_ILUTS
	Integer*4	IPC_PNAMES
	Integer*4	IPC_MPIOSAVE
	Integer*4	IPC_MPIOLOAD
	Parameter	(IPC_QUIT = -1)
	Parameter	(IPC_OK   =  0)
	Parameter	(IPC_POINT=  1)
	Parameter	(IPC_CLEAR=  2)
	Parameter	(IPC_VLUTS=  3)
	Parameter	(IPC_ILUTS=  4)
	Parameter	(IPC_PNAMES=  5)
	Parameter	(IPC_MPIOSAVE=  6)
	Parameter	(IPC_MPIOLOAD=  7)
C
C	Packet format:
C	(Packet type)(additonal data) = size in longs => destination app
C
C	(IPC_PNAMES) = 1 => Voxel
C	Reread pointset files for new names, one has changed
C
C	(IPC_MPIOLOAD)(256byte string) = 65 => Voxel
C	Load the specified MP_FIT settings file
C
C	(IPC_MPIOSAVE)(256byte string) = 65 => Voxel
C	Save the MP_FIT settings to the specified file
C
C	(IPC_POINT) = 1 => Trace
C	Sending 2D points to trace via pts_addr shared memory
C
C	(IPC_CLEAR) = 1 => Trace
C	Clear all current 2D points
C
C	(IPC_ILUTS) = 1 => Trace
C	Reread the "_ilut" file (obsolete)
C
C	(IPC_VLUTS) = 1 => Voxel
C	Reread the "_vlut","_opacity", and "_ilut" files (obsolete)
C
C	Added for lighting model dialog
C
	Integer*4	IPC_L_INTRINSIC
	Parameter	(IPC_L_INTRINSIC=  8)
	Integer*4	IPC_L_FILTER
	Parameter	(IPC_L_FILTER=  9)
	Integer*4	IPC_L_POSTLIGHT
	Parameter	(IPC_L_POSTLIGHT= 10)
C
C	(IPC_L_INTRINSIC)(ONOFF)(DX,DY,DZ,INT1,INT2) = 7 => Voxel
C	Set the intrinsic lighting model settings
C
C	(IPC_L_POSTLIGHT)(DX,DY,DZ,INT) = 5 => Voxel
C	Postlight the image
C
C	(IPC_L_FILTER)(FILTERNUM) = 2 => Voxel
C	Filter the current image
C
C	Added for new palette editor model
C
	Integer*4	IPC_V_VLUT
	Integer*4	IPC_V_ILUT
	Integer*4	IPC_T_ILUT
	Integer*4	IPC_V_TLUT
	Integer*4	IPC_T_TLUT
	Parameter	(IPC_V_VLUT= 11)
	Parameter	(IPC_V_ILUT= 12)
	Parameter	(IPC_T_ILUT= 13)
	Parameter	(IPC_V_TLUT= 14)
	Parameter	(IPC_T_TLUT= 28)
C
C	(IPC_T_ILUT)(256*I4 AABBGGRR) = 257 => Trace
C	New RGBA 2D Image lut
C
C	(IPC_T_TLUT)(256*I4 AABBGGRR) = 257 => Trace
C	New RGBA Texture lut
C
C	(IPC_V_VLUT)(256*I4 AABBGGRR) = 257 => Voxel
C	New RGBA Volume lut
C
C	(IPC_V_ILUT)(256*I4 AABBGGRR) = 257 => Voxel
C	New RGBA 2D Image lut
C
C	(IPC_V_TLUT)(256*I4 AABBGGRR) = 257 => Voxel
C	New RGBA Texture lut
C
C	Added for the filer interface
C
	Integer*4	IPC_F_RETURN
	Parameter	(IPC_F_RETURN= 15)
C
C	See filer/filer_inc.inc for details
C
C	Added for the bitslice volume loading interface
C
	Integer*4	IPC_P_BITSLICE
	Parameter	(IPC_P_BITSLICE= 29)
C
C	(IPC_P_BITSLICE)(volnum 0=mri 1=texture)(flags)(min)(max) = 5 => paled
C	flags = #bits + (256 if max/min scaled)
C
C	Added for the histogram passing interface
C
	Integer*4	IPC_I_HISTO
	Parameter	(IPC_I_HISTO= 16)
C
C	(IPC_I_HISTO)(256*I4 histogram)(histnum) = 258 => paled
C	histnum: 0=MRIvolume,1=cutplane,2=texturevolume,3=cuttexture
C	Here is a new histogram...
C
	Integer*4       IPC_H_MRICUT
	Parameter       (IPC_H_MRICUT= 1)	
	Integer*4       IPC_H_MRIVOL
	Parameter       (IPC_H_MRIVOL= 0)	
	Integer*4       IPC_H_TEXTURECUT
	Parameter       (IPC_H_TEXTURECUT= 3)	
	Integer*4       IPC_H_TEXTUREVOL
	Parameter       (IPC_H_TEXTUREVOL= 2)	
C
C	Added for texture functions 
C
	Integer*4	IPC_TEXFUNCS
	Parameter	(IPC_TEXFUNCS= 17)
	Integer*4	IPC_TEXVOLUME
	Parameter	(IPC_TEXVOLUME= 18)
	Integer*4	IPC_TEXCLIP
	Parameter	(IPC_TEXCLIP= 19)
	Integer*4	IPC_TEXFILL
	Parameter	(IPC_TEXFILL= 20)
	Integer*4	IPC_TEXUTIL
	Parameter	(IPC_TEXUTIL= 21)
	Integer*4	IPC_2D_TEXFILL
	Parameter	(IPC_2D_TEXFILL= 22)
C
C	(IPC_TEXFUNCS) = 1 => Voxel
C	Reread the texture functions file.
C
C	(IPC_TEXVOLUME) = 1 => Voxel
C	A new texture volume has been loaded.
C
C	(IPC_TEXCLIP)(type)(value) = 3 => Voxel
C	(IPC_TEXFILL)(type)(value) = 3 => Voxel
C	(IPC_2D_TEXFILL)(type)(value)(roinum) = 4 => Trace
C	Perform one of the following operations(type):
	Integer*4       IPC_TEX_CL_PAINT
	Parameter       (IPC_TEX_CL_PAINT= 0)
	Integer*4       IPC_TEX_CL_TAG
	Parameter       (IPC_TEX_CL_TAG= 1)
	Integer*4       IPC_TEX_CL_UNTAG
	Parameter       (IPC_TEX_CL_UNTAG= 2)
C
C	Exclusive to IPC_2D_TEXFILL
C
	Integer*4       IPC_TEX_CL_INCR		
	Parameter       (IPC_TEX_CL_INCR= 3)
	Integer*4       IPC_TEX_CL_DECR		
	Parameter       (IPC_TEX_CL_DECR= 4)
C
C	(IPC_TEXUTIL)(type)(value1)(value2) = 4 => Voxel
C	Perform one of the following operations:
	Integer*4       IPC_TEX_UTIL_MULTC
	Parameter       (IPC_TEX_UTIL_MULTC= 0)
	Integer*4       IPC_TEX_UTIL_MULTV
	Parameter       (IPC_TEX_UTIL_MULTV= 1)
	Integer*4       IPC_TEX_UTIL_SWAP
	Parameter       (IPC_TEX_UTIL_SWAP= 2)
	Integer*4       IPC_TEX_UTIL_ADDC
	Parameter       (IPC_TEX_UTIL_ADDC= 3)
	Integer*4       IPC_TEX_UTIL_ADDV
	Parameter       (IPC_TEX_UTIL_ADDV= 4)
	Integer*4       IPC_TEX_UTIL_VOL2TEX
	Parameter       (IPC_TEX_UTIL_VOL2TEX= 5)
	Integer*4       IPC_TEX_UTIL_TEX2VOL
	Parameter       (IPC_TEX_UTIL_TEX2VOL= 6)
	Integer*4       IPC_TEX_UTIL_FFILL
	Parameter       (IPC_TEX_UTIL_FFILL= 7)
	Integer*4       IPC_TEX_UTIL_REBOUND
	Parameter       (IPC_TEX_UTIL_REBOUND= 8)
C
C	Added to get the current arb plane data from voxel
C
	Integer*4	IPC_T_ARBREQUEST
	Parameter	(IPC_T_ARBREQUEST = 23)
	Integer*4	IPC_T_ARBREPLY
	Parameter	(IPC_T_ARBREPLY = 24)
C
C	(IPC_T_ARBREQUEST) = 1 => Voxel
C	Trace is asked for the current arb rotation and D values
C	(IPC_T_ARBREPLY)(valid)(rx)(ry)(rz)(D) = 6 => Trace
C	Returns the rotations and D value for current voxel image 
C		valid only is (valid)=1, otherwise, not in arb mode.
C
C	Private (internal) messages (defined by each sub-app)
C
	Integer*4	IPC_TRACE_PRIVATE
	Parameter	(IPC_TRACE_PRIVATE = 25)
	Integer*4	IPC_VOXEL_PRIVATE
	Parameter	(IPC_VOXEL_PRIVATE = 26)
	Integer*4	IPC_PALS_PRIVATE
	Parameter	(IPC_PALS_PRIVATE = 27)
C
C	Next IPC is 30
C
C	lookup tables
C
	Integer*4	llut(768)   ! one side of plane
	Integer*4	rlut(768)   ! other side of plane
	Integer*4	ilut(768)   ! image lut

	common 	/luts/llut,rlut,ilut
C
C	general parameters
C
	Character*15	version,com_date

	parameter	(com_date = '1996-2006')
	parameter	(version = '3.14(12/09/06)')
C
