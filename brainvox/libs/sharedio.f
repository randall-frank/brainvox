C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C	Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : sharedio
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 19 Nov 1990    
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	these routines allow for access to the data stored
C			in shared memory.  They include parsing of the
C			std data as well as image access,lut access, and
C			shared mem IPC.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C    LIMITATIONS    :      
C    NOTES          :  See each of the routines
C
C                 
C    ====================================================================== 
C
C	creates the shared memory blocks                   
C	It should be called by all program as soon as they have the
C	passed shared memory ID to the data segment (shm_data)
C
	Subroutine	vt_init_shmem

	Implicit None

	include 'vt_inc.f'  

	Integer*4	key,bv_getpid,i

	external	bv_getpid
C
C	check to see if you are the shared memory owner
C
	if (shmem_owner .eq. 0) goto 100
C
C	If you are the owner then allocate the memory
C	otherwise mount the memory assuming that 
C	shm_data is correct.
C
C	Compute the amount of memory required for the images
C
	IMAGES_SHM_SIZE = imagesize*imagesize*(realimages + 
     +			((realimages-1)*interpimages))
C
C	due to errors in user specified templates I detect the no
C	image case and force it to allocate some memory
C
	if (IMAGES_SHM_SIZE .le. 0) IMAGES_SHM_SIZE = 1
C
C	key = bv_getpid()
C
C	new key system: start at my B-day and increment by 10 until
C	no valid segment exists
C
	key = 267110 ! $41366
	Call get_next_key(key,%val(10))
C
	Call create_shared_mem(%val(key),%val(DATA_SHM_SIZE),shm_data)

	i = ((4*(IPC_SHM_SIZE*2))*20)+(4*3)
	key = key + 1
	Call create_shared_mem(%val(key),%val(i),shm_ipc)

	key = key + 1
	Call create_shared_mem(%val(key),%val(IMAGES_SHM_SIZE),
     +		shm_images)

	key = key + 1
	Call create_shared_mem(%val(key),%val(PTS_SHM_SIZE),shm_pts)

	key = key + 1
	Call create_shared_mem(%val(key),%val(IMAGES_SHM_SIZE),
     +		shm_texture)
C
C	clear all mount points for memory  (ie none are attached)
C
	pts_addr = 0
	data_addr = 0
	ipc_addr = 0
	images_addr = 0
	texture_addr = 0
	lut_addr = 0
C
C	attach to all shared memory
C
	call attach_shared_mem(%val(shm_data),data_addr)
	lut_addr = data_addr + LUT_SHM_OFFSET
	call attach_shared_mem(%val(shm_ipc),ipc_addr)
	call attach_shared_mem(%val(shm_images),images_addr)
	call attach_shared_mem(%val(shm_pts),pts_addr)
	call attach_shared_mem(%val(shm_texture),texture_addr)
C
C	initialize the shm_data block
C
	Call vt_write_data
C
C	initialize the IPC queue (may need more than this) 
C
	Call init_IPC(1,%val(ipc_addr))
C
	return
C
100	continue  ! not the owner
C
C	clear all mount points for memory  (ie none are attached)
C
	pts_addr = 0
	data_addr = 0
	ipc_addr = 0
	images_addr = 0
	texture_addr = 0
	lut_addr = 0
C
C	attach to all shared memory in two steps
C
	call attach_shared_mem(%val(shm_data),data_addr)
	lut_addr = data_addr + LUT_SHM_OFFSET
C
C	if you are not the owner then mount the shm_data segment
C	and read it.  This validates the other shm_* vars and
C	you can then attach them
C
	Call vt_read_data
C
C	now the other shm's are valid and may be attached
C
	call attach_shared_mem(%val(shm_ipc),ipc_addr)
	call attach_shared_mem(%val(shm_images),images_addr)
	call attach_shared_mem(%val(shm_pts),pts_addr)
	call attach_shared_mem(%val(shm_texture),texture_addr)
C
C	get the messaging started (0=not owner)
C
	Call init_IPC(0,%val(ipc_addr))
C
	return
	end
C
C	Routine to dump all shared memory  (QUIT)
C
C	Note: this first tells everyone via IPC to get their fingers
C		out and then shuts the door when they are all out.
C
C	This routine should be called by all programs on their exit
C
	Subroutine	vt_quit_shmem

	Implicit None

	include 'vt_inc.f'  

        Integer*4 bv_os
	External bv_os

	Character*80	tstr
	Integer*4	numusers,corenumusers,key,size,i,status
C
C	if you own it??
C
	if (shmem_owner .eq. 1) goto 1000
C
C	detach to all shared memory
C
	call detach_shared_mem(%val(data_addr),status)
	call detach_shared_mem(%val(ipc_addr),status)
	call detach_shared_mem(%val(images_addr),status)
	call detach_shared_mem(%val(pts_addr),status)
	call detach_shared_mem(%val(texture_addr),status)
C
C	if you don't own the memory then return
C
	return

1000	Continue
C
C	tell everyone left to get out...
C
C	Call send_IPC(1,IPC_QUIT)  ; Interesting Error RJF
C
C	detach to all shared memory (get out yourself)
C
	call detach_shared_mem(%val(data_addr),status)
	call detach_shared_mem(%val(ipc_addr),status)
	call detach_shared_mem(%val(images_addr),status)
	call detach_shared_mem(%val(pts_addr),status)
	call detach_shared_mem(%val(texture_addr),status)
C
C	As soon as there are no other attached to the segment, dump it.
C
	numusers = 1
	Do while ((numusers .ne. 0).and.(bv_os() .ne. 0))
		Call status_shared_mem(%val(shm_ipc),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
	Call delete_shared_mem(%val(shm_ipc),status)
C
C
	numusers = 1
	Do while ((numusers .ne. 0).and.(bv_os() .ne. 0))
		Call status_shared_mem(%val(shm_data),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
	Call delete_shared_mem(%val(shm_data),status)
C
C
	numusers = 1
	Do while ((numusers .ne. 0).and.(bv_os() .ne. 0))
		Call status_shared_mem(%val(shm_images),size,key,
     +			numusers,corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
	Call delete_shared_mem(%val(shm_images),status)
C
C
	numusers = 1
	Do while ((numusers .ne. 0).and.(bv_os() .ne. 0))
		Call status_shared_mem(%val(shm_pts),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
	Call delete_shared_mem(%val(shm_pts),status)
C
C
	numusers = 1
	Do while ((numusers .ne. 0).and.(bv_os() .ne. 0))
		Call status_shared_mem(%val(shm_texture),size,key,  
     +			numusers,corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
	Call delete_shared_mem(%val(shm_texture),status)
C
	Return
	End
C
C	child quit routine (used only by main program to wait for
C	spawned processes to terminate)
C	this is called in response to a IPC_QUIT ipc from a child
C
	Subroutine	vt_child_wait

	Implicit None

	include 'vt_inc.f'  

	Integer*4	numusers,corenumusers,key,size,i,status
C
C	if you own it?? (this is for owners only!!)
C
	if (shmem_owner .ne. 1) return    
C
C	wait for an IPC_QUIT to be generated by a child
C	(or five seconds)
C
	i = IPC_OK
	status = 0
	Do while ((i .ne. IPC_QUIT).and.(status .lt. 20))
		size = 1
		Call check_ipc(size,i)
		if (i .ne. IPC_QUIT) call bv_sginap(1000) !1000ticks=1sec
		status = status + 1
	Enddo
C
C	send an IPC_QUIT ourselves.
C	in case a driver died but other connectors are still running
C
	If (status .gt. 19) then
		Call send_ipc(1,IPC_OK)
		Call send_ipc(1,IPC_QUIT)
	Endif
C
C	detach to all shared memory (get out yourself)
C
C	call detach_shared_mem(%val(data_addr),status)
C	call detach_shared_mem(%val(ipc_addr),status)
C	call detach_shared_mem(%val(images_addr),status)
C	call detach_shared_mem(%val(pts_addr),status)
C	call detach_shared_mem(%val(texture_addr),status)
C
C	As soon as there are no other users
C	attached to the segment, attach yourself to them all.
C
	numusers = 2
	Do while (numusers .gt. 1)
		Call status_shared_mem(%val(shm_ipc),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
C
C
	numusers = 2
	Do while (numusers .gt. 1)
		Call status_shared_mem(%val(shm_data),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
C
C
	numusers = 2
	Do while (numusers .gt. 1)
		Call status_shared_mem(%val(shm_images),size,key,
     +			numusers,corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
C
C
	numusers = 2
	Do while (numusers .gt. 1)
		Call status_shared_mem(%val(shm_pts),size,key,numusers,
     +			corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
C
C
	numusers = 2
	Do while (numusers .gt. 1)
		Call status_shared_mem(%val(shm_texture),size,key,
     +			numusers,corenumusers,i,status)
		Call bv_sginap(100)
	Enddo
C
C	attach to all shared memory (again)
C
C	call attach_shared_mem(%val(shm_data),data_addr)
C	call attach_shared_mem(%val(shm_ipc),ipc_addr)
C	call attach_shared_mem(%val(shm_images),images_addr)
C	call attach_shared_mem(%val(shm_pts),pts_addr)
C	call attach_shared_mem(%val(shm_texture),texture_addr)
C
C	clear the IPC queue
C
	Call send_IPC(1,IPC_OK)
C
	Return
	End
C
C------------ Data storage routines ----
C
C
C	writes the current data block into shared memory
C	It stores the current state of all patient infor into shared memory
C
	Subroutine	vt_write_data

	Implicit None

	include 'vt_inc.f'  
C
C	input and output via common block
C
	Call vt_data_io_sub(IPC_WRITE,%val(data_addr))
C
	return
	end
C
C	Reads the data block from shared memory
C	It reads the current state of all patient info from shared memory
C
	Subroutine	vt_read_data

	Implicit None

	include 'vt_inc.f'  
C
C	input and output via common block
C
	Call vt_data_io_sub(IPC_READ,%val(data_addr))

	return
	end
C
C	Real data block memory handling routine
C	This routine actually implements the vt_xxxx_data calls
C	and provides a needed chance to dereference data_addr
C
	Subroutine	vt_data_io_sub(cmd,addr)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	cmd,addr(*)
	Integer*4	i,ptr

	If (cmd .eq. IPC_READ) then
C
C	get the common block data vt and shared_mem from addr()
C
C	first /shared_mem/
C
		shm_ipc = addr(1)
		shm_data = addr(2)
		shm_pts = addr(3)
		shm_images = addr(4)
C
C	Note: shmem_owner and *_addr are NOT copied
C		These are provided by the calling program or inited
C		via an attach_shared_mem call.
C
C	Now /vt/
C
		realimages = addr(5)
		imagesize = addr(6)
		interpimages = addr(7)
		numrgns = addr(8)
		ptr = 9
		Do i=1,max_rgns
			rgntypes(i) = addr(ptr)
			ptr = ptr + 1
		Enddo
		Do i=1,max_rgns
			Call data_copy_string(addr(ptr),40,rgnnames(i))
			ptr = ptr + (40/4)
		Enddo
		Call data_copy_string(addr(ptr),200,imagename)  
		ptr = ptr + (200/4)
		Call data_copy_string(addr(ptr),200,patientinfo(1))
		ptr = ptr + (200/4)
		Call data_copy_string(addr(ptr),200,patientinfo(2))
		ptr = ptr + (200/4)
		Call data_copy_string(addr(ptr),200,patientinfo(3))
		ptr = ptr + (200/4)
		Call data_copy_real(addr(ptr),interpixel) 
		ptr = ptr + 1
		Call data_copy_real(addr(ptr),interslice) 
		ptr = ptr + 1
		Call data_copy_real(addr(ptr),squeezefactor)
		ptr = ptr + 1
		cut_dir = addr(ptr)
		ptr = ptr + 1
		volumedepth = addr(ptr)
		ptr = ptr + 1
		shm_texture = addr(ptr)
		ptr = ptr + 1
		brainvox_options = addr(ptr)
		ptr = ptr + 1
		Call data_copy_real(addr(ptr),volume_lims(1))
		ptr = ptr + 1
		Call data_copy_real(addr(ptr),volume_lims(2))
		ptr = ptr + 1
		volume_zswap = addr(ptr)
		ptr = ptr + 1
		volume_bswap = addr(ptr)
		ptr = ptr + 1
C
	Else if (cmd .eq. IPC_WRITE) then
C
C	write the common block data vt and shared_mem into addr()
C
C	first /shared_mem/
C
		addr(1) = shm_ipc
		addr(2) = shm_data 
		addr(3) = shm_pts
		addr(4) = shm_images 
C
C	Now /vt/
C
		addr(5) = realimages
		addr(6) = imagesize
		addr(7) = interpimages
		addr(8) = numrgns
		ptr = 9
		Do i=1,max_rgns
			addr(ptr) = rgntypes(i)
			ptr = ptr + 1
		Enddo
		Do i=1,max_rgns
			Call data_copy_string(rgnnames(i),40,addr(ptr))
			ptr = ptr + (40/4)
		Enddo
		Call data_copy_string(imagename,200,addr(ptr))  
		ptr = ptr + (200/4)
		Call data_copy_string(patientinfo(1),200,addr(ptr))
		ptr = ptr + (200/4)
		Call data_copy_string(patientinfo(2),200,addr(ptr))
		ptr = ptr + (200/4)
		Call data_copy_string(patientinfo(3),200,addr(ptr))
		ptr = ptr + (200/4)
		Call data_copy_real(interpixel,addr(ptr)) 
		ptr = ptr + 1
		Call data_copy_real(interslice,addr(ptr)) 
		ptr = ptr + 1
		Call data_copy_real(squeezefactor,addr(ptr)) 
		ptr = ptr + 1
		addr(ptr) = cut_dir
		ptr = ptr + 1
		addr(ptr) = volumedepth
		ptr = ptr + 1
		addr(ptr) = shm_texture
		ptr = ptr + 1
		addr(ptr) = brainvox_options
		ptr = ptr + 1
		Call data_copy_real(volume_lims(1),addr(ptr)) 
		ptr = ptr + 1
		Call data_copy_real(volume_lims(2),addr(ptr)) 
		ptr = ptr + 1
		addr(ptr) = volume_zswap
		ptr = ptr + 1
		addr(ptr) = volume_bswap
		ptr = ptr + 1
C
	Endif

	Return
	End
C
C	routine to copy a byte string from source to dest
C	Note: source or dest could very well be a contrived pointer
C		this is why this routine is needed
C
	subroutine	data_copy_string(source,len,dest)

	Implicit None

	Character*(*)	source,dest
	Integer*4	len,i

	do I=1,len
		dest(i:i) = source(i:i)
	Enddo

	return
	End
C
C	routine to copy a real from source to dest
C	Note: source or dest could very well be a contrived pointer
C		this is why this routine is needed
C
	subroutine	data_copy_real(source,dest)

	Implicit None

	Real*4		source,dest
C
C	Ooo! So much work for a simple assignment.
C
	dest = source

	return
	End
C
C------------ Image storage routines ----
C
C	Note: these routines allow access only to the REAL images (no interps)
C
C	reads an image from shared memory
C
	Subroutine	vt_read_realimage(num,image)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	num
	byte		image(*)
C
C	and once again we call the real routine
C
	Call vt_image_io_sub(IPC_READ,num,image,%val(images_addr))

	Return
	End
C
C	writes an image into shared memory
C
	Subroutine	vt_write_realimage(num,image)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	num
	Byte		image(*)
C
	Call vt_image_io_sub(IPC_WRITE,num,image,%val(images_addr))
C
	Return
	End
C
C	this routine does all the real work
C
	Subroutine	vt_image_io_sub(cmd,num,image,addr)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	cmd,num
	Integer*4	ptr,i,isize
	Byte		image(*)
	Byte		addr(*)
C
C	get byte pointer
C	ptr = (size of image)*(image index-1)*(1 image + numinterps)
C
	isize = imagesize*imagesize
	ptr = ((isize)*(num-1)*(1+interpimages))
C
C	routine select
C
	If (cmd .eq. IPC_READ) then
		Do I=1,isize
			image(i) = addr(i+ptr)
		Enddo
	Else if (cmd .eq. IPC_WRITE) then
		Do I=1,isize
			addr(i+ptr) = image(i)
		Enddo
	Endif

	return
	End
C
C------------ LUT RGBA lookups --
C
C	Read a texture function map
C
	Subroutine	vt_read_lut_map(which,map)

	Implicit None

	Include 'vt_inc.f'

	Integer*4       which
	Integer*4	map(256)
	Integer*4	i

	If (which .eq. 1) Then
		i = LUT_IMG_OFFSET
	Else If (which .eq. 2) Then
		i = LUT_VXL_OFFSET
	Else If (which .eq. 3) Then
		i = LUT_TEX_OFFSET
	Endif

	Call vt_memcpy_io_sub(IPC_READ,map,256*4,%val(lut_addr+i))

	Return
	End
C
C	Write a texture function map
C
	Subroutine	vt_write_lut_map(which,map)

	Implicit None

	Include 'vt_inc.f'

	Integer*4       which
	Integer*4	map(256)
	Integer*4	i

	If (which .eq. 1) Then
		i = LUT_IMG_OFFSET
	Else If (which .eq. 2) Then
		i = LUT_VXL_OFFSET
	Else If (which .eq. 3) Then
		i = LUT_TEX_OFFSET
	Endif

	Call vt_memcpy_io_sub(IPC_WRITE,map,256*4,%val(lut_addr+i))

	Return
	End
C
C------------ Texture function lookups --
C
C	Read a texture function map
C
	Subroutine	vt_read_tex_fun_map(which,map)

	Implicit None

	Include 'vt_inc.f'

	Integer*4	which
	Integer*4	map(65536)
	Integer*4	i,j

	If (which .eq. 1) then
		i = LUT_FN1_OFFSET
	Else if (which .eq. 2) then
		i = LUT_FN2_OFFSET
	Else
		Do j=0,256
		Do i=0,256
			map(i+(j*256)+1) = i + (i*256) + (i*65536)
		Enddo
		Enddo

		Return
	Endif

	Call vt_memcpy_io_sub(IPC_READ,map,65536*4,%val(lut_addr+i))

	Return
	End
C
C	write a texture function map
C
	Subroutine	vt_write_tex_fun_map(which,map)

	Implicit None

	Include 'vt_inc.f'

	Integer*4	which
	Integer*4	map(65536)
	Integer*4	i

	If (which .eq. 1) then
		i = LUT_FN1_OFFSET
	Else if (which .eq. 2) then
		i = LUT_FN2_OFFSET
	Else
		Return
	Endif

	Call vt_memcpy_io_sub(IPC_WRITE,map,256*256*4,%val(lut_addr+i))

	Return
	End
C
C	Generic shared memory copy routine
C
	Subroutine vt_memcpy_io_sub(cmd,data,len,addr)
	
	Implicit None

	Include 'vt_inc.f'

	Integer*4	cmd,len
	Byte		data(*),addr(*)

	If (cmd .eq. IPC_READ) Then
		Call memcopy(data,addr,len)
	Else if (cmd .eq. IPC_WRITE) Then
		Call memcopy(addr,data,len)
	Endif

	Return
	End
C
C------------ Point storage routines ----
C
C	reads a point set from shared memory
C
	Subroutine	vt_read_points(num,points)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	num
	Integer*4	points(4096,4)
C
C	and once again we call the real routine
C
	Call vt_point_io_sub(IPC_READ,num,points,%val(pts_addr))

	Return
	End
C
C	writes a point set into shared memory
C
	Subroutine	vt_write_points(num,points)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	num
	Integer*4	points(4096,4)
C
	Call vt_point_io_sub(IPC_WRITE,num,points,%val(pts_addr))
C
	Return
	End
C
C	this routine does all the real work
C
	Subroutine	vt_point_io_sub(cmd,num,points,addr)

	Implicit None

	include 'vt_inc.f'  

	Integer*4	cmd,num,points(4096,4)
	Integer*4	i,j
	Integer*4	addr(4097,4)
C
C	routine select
C
	If (cmd .eq. IPC_READ) then
		num = addr(4097,1)
		Do I=1,num
			Do J=1,4
				points(i,j) = addr(i,j)
			Enddo
		Enddo
	Else if (cmd .eq. IPC_WRITE) then
		addr(4097,1) = num
		Do I=1,num
			Do J=1,4
				addr(i,j) = points(i,j)
			Enddo
		Enddo
	Endif

	return
	End
C
C	Utility routines
C
C	Routine to look for environmental options
C
        Subroutine bv_get_options

        Implicit None

	Include 'vt_inc.f'

	Character*500	env

        brainvox_options = OPTS_NONE

	Call getenv("BRAINVOX_OPTIONS",env)

	If (index(env,"ADVANCED") .ne. 0) Then
		brainvox_options = brainvox_options + OPTS_ADVANCED
	Endif
	If (index(env,"TESTING") .ne. 0) Then
		brainvox_options = brainvox_options + OPTS_TESTING
	Endif
	If (index(env,"RJF") .ne. 0) Then
		brainvox_options = brainvox_options + OPTS_RJF
	Endif

        Return
        End
C
C	Function to check an option
C
	Integer*4 Function bv_test_option(opt)

	Implicit None

	Include 'vt_inc.f'

	Integer*4 opt

	If (iand(opt,brainvox_options) .ne. 0) Then
		bv_test_option = 1
	Else
		bv_test_option = 0
	Endif

	Return
	End
