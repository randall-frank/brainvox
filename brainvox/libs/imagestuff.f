C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C       Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : imagestuff
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 26 Nov 90       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This file contains routines to zoom/convert and flip
C			images.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C    INPUTS         :        
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine vt_zoom_image(I1,image)

	Implicit None

	Include 'vt_inc.f'
	
	Integer*4	ii,kk,k
	Integer*4	x,y,ptr,mult
C
	Integer*2	I1(512*512)
	Byte		image(*)
C
C	Handle the file.  Convert to 512x512 and to I*2
C
C	ptr is index into output image
C	mult is the square repeat factor
C
	ptr = 1
	mult = 512/imagesize
C
C	over all scan line in source
C
	Do y = 0, imagesize-1
C
C	repeat scan lines
C
	  Do kk = 1, mult
C
C	over a single scan line in the source
C
	    Do x = 1, imagesize
		ii = image(x + (y*imagesize))
		if (ii .lt. 0) ii = 256 + ii
		ii = ii + 256
C
C	repeat pixels along the scan line
C
		Do k = 1, mult
			I1(ptr) = ii
			ptr = ptr + 1
		Enddo  ! next pixel repeat
	    Enddo   ! next pixel
	  Enddo  ! next scan line repeat
	Enddo  ! next scan line  
C
	Return
	End
C
	subroutine	flipbyteimage(image)
C
C	routine to vertically flip a byte image...
C
	Implicit None

	Include 'vt_inc.f'

	byte		image(*),tmp

	Integer*4	i,jj,II,k,line
C
C	flip the image vertically for the SG screen
C
	line = imagesize
	Do II=1,line/2
		K=(II-1)*line
		I=(line-II)*line
		Do JJ=1,line
			tmp=image(k+jj)
			image(k+jj)=image(I+jj)
			image(I+jj)=tmp
		Enddo
	Enddo
C
	return
	End
C
C  This routine loads a set of images into shared memory
C  and computes the interpolated images
C
	Subroutine loadimages(error)

	Implicit None

	Include 'vt_inc.f'

	Integer*4	id  !  0=hemis  1=right  2=left

	byte		buffer(512*512)
	logical		error
	Character*100	tname,iname,sname,dirname,roi
	Integer*4	i,start,iend,isize,stepsize,dest,j,out_slice
	Integer*4	strlen
	External	strlen
	Integer*4	bv_nifti_io
	External bv_nifti_io
	Integer*4	ndx,ndy,ndz,slice,dz,flip,isnifti
	Real*4		is,ip
C
C	NIFTI stuff
C
	isnifti = bv_nifti_io(imagename(1:strlen(imagename)),2,0,
     +			ndx,ndy,ndz,slice,dz,flip,ip,is)
C
	id = cut_dir
C
C	Get the directory base
C
	Call indexnum_to_roi(id,roi,i)
	Call roi_to_dir(roi,dirname)
C
C	Compute the squeeze factor for this dataset
C
        squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)
C
C	try all the images
C
	Do i=1,realimages
C
		call draw_status_bar(float(i)/float(realimages),
     +			"Loading images...}")
C
C	get the image name
C
		Call get_slice_num(i,out_slice)
		If (isnifti .eq. 0) Then
			tname = imagename(1:strlen(imagename))
		Else
			tname = " "
			Call name_changer(imagename,out_slice,tname,error)
			if (error) goto 999
		Endif
C
C	find simple version of image name
C
		j = 100
		Do while((j.gt.0).and.(tname(j:j) .ne. '/'))
			j = j - 1
		Enddo
		sname=tname(j+1:100)
C
C	drop down a directory if requested
C		
		if (id .eq. 0) then
			iname = tname
C		Else if (id .eq. 1) then
C			iname = "righthemi/"//sname
C		Else if (id .eq. 2) then
C			iname = "lefthemi/"//sname
		Else 
			iname = dirname(1:strlen(dirname))//"/"//
     +				sname
		Endif
		If (isnifti .eq. 0) Then
			error = .false.
			if (bv_nifti_io(iname(1:strlen(iname)),0,buffer,
     +				ndx,ndy,ndz,out_slice,dz,
     +				flip,ip,is) .ne. 0) Then
				error = .true.
			Endif
		Else
			Call read_image(iname,buffer,error)
		Endif
C
C	if there is an error try again higher up if possible
C
		If (error) then
		   write(*, 10)I
10	Format("Unable to read image ",i3
     +			," on first attempt.  Trying again.")
		   If (isnifti .eq. 0) Then
			error = .false.
			if (bv_nifti_io(tname(1:strlen(tname)),0,buffer,
     +				ndx,ndy,ndz,out_slice,dz,
     +				flip,ip,is) .ne. 0) Then
				error = .true.
			Endif
		   Else
			Call read_image(tname,buffer,error)
		   Endif
		   if (error) goto 999
		Endif
C
C	flip the image over the X axis
C
		Call flipbyteimage(buffer)
C
C	copy the image into shared memory
C
		Call vt_write_realimage(I,buffer)
	Enddo
C
C	now interpolate the images
C
	start = 0
	isize = (imagesize*imagesize)
	stepsize = (interpimages+1)*isize
C
	Do i=1,realimages-1
C
C	status bar
C
		call draw_status_bar(float(i)/float(realimages-1),
     +			"Interpolating images...}")
C
C	interpolate the images
C
		iend = start + stepsize
		dest = start + isize
C
C  vl_interp_images_(char *start, char *end, int x,int y,int num,char *dest)
C
		Call vl_interp_images(%val(images_addr+start),
     +				      %val(images_addr+iend),
     +				      %val(imagesize),%val(imagesize),
     +				      %val(interpimages),
     +				      %val(images_addr+dest))
C
C	bump start along
C
		start = start + stepsize
C
	Enddo
C
	error =.false. 
C
	Return

999	write(*, 998)i,imagename
998	Format("Unable to read image number:",I5," Using template:",
     +		/,A40)
	error = .true.

	return
	End
C
C	routine to draw a box showing 
C
	subroutine draw_status_bar(percent,string)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'

	real*4	percent
	Integer*4	x,y,x1,y1,x2,y2
	Character*(*)   string

	Integer*4	iDB, iRGB, h

	Call getsiz(x,y)
	Call reshap
	
	If ((getdis() .eq. DMRGB) .or. (getdis() .eq. DMRGBD)) then
		iRGB = 1
	Else
		iRGB = 0
	Endif
	If ((getdis() .eq. DMSING) .or. (getdis() .eq. DMRGB)) then
		iDB = 0
	Else
		iDB = 1
	Endif
	
	If (iRGB .eq. 1) then
		h = '00545454'X
		Call cpack(h)
	Else
		Call color(BLACK)
	Endif
C
C	If first time, or double buffered, then  clear!!!
C
	If ((percent .lt. 0.001).or.(iDB .eq. 1)) Then
		Call clear
	Endif
C
C	the title 
C
	If (iRGB .eq. 1) then
		h = '00ffffff'X
		Call cpack(h)
	Else
		Call color(WHITE)
	Endif
	Call cmov2i(10,y-25)
	Call charst(string,index(string,"}")-1)
C
C	assume the viewport is 1 to 1
C
	x1 = 10
	x2 = x - 10
	y1 = 10
	y2 = y1 + 20
	If (iRGB .eq. 1) then
		h = '00AAAA00'X
		Call cpack(h)
	Else
		Call color(GREEN)
	Endif
	call rectfi(x1,y1,x2,y2)
	If (iRGB .eq. 1) then
		Call cpack(0)
	Else
		Call color(BLACK)
	Endif
	call recti(x1-1,y1-1,x2,y2)

	x1 = x1 + 3
	x2 = x2 - 3
	y1 = y1 + 3
	y2 = y2 - 3
	x2 = jifix(float(x2-x1)*percent)+x1
	If (iRGB .eq. 1) then
		h = '00333300'X
		Call cpack(h)
	Else
		Call color(BLUE)
	Endif
	call rectfi(x1,y1,x2,y2)
C
	If (iDB) then
		call swapbu
	Else 
		call gflush
	Endif
C	
	return
	end
C
C  This routine loads a set of images into memory
C  and computes the interpolated images
C
C  Uses the following from /globals/:
C	imagename - template
C	realimages - number of images
C	interslice,interpixel - obvious
C
	Subroutine background_loadimages(error)

	Implicit None

	Include 'vt_inc.f'

	logical		error
	byte		buffer(512*512)
	Character*100	tname,iname,sname,dirname,roi
	Integer*4	i,start,iend,isize,stepsize,dest,j,out_slice
	Integer*4	strlen
	External	strlen
	Integer*4	bv_nifti_io
	External bv_nifti_io
	Integer*4	ndx,ndy,ndz,slice,dz,flip,isnifti
	Real*4		is,ip
C
C	NIFTI stuff
C
	isnifti = bv_nifti_io(imagename(1:strlen(imagename)),2,0,
     +			ndx,ndy,ndz,slice,dz,flip,ip,is)
C
C	Compute the squeeze factor for this dataset
C
        squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)
C
C	try all the images
C
	write(*, *)"Loading images... "
	Do i=1,realimages
C
C	get the image name
C
		tname = " "
		Call get_slice_num(i,out_slice)
		Call name_changer(imagename,out_slice,iname,error)
		If (error) Return
C
		Do j=1,(imagesize*imagesize)
			buffer(j) = 0
		Enddo
		if (isnifti .eq. 0) then
		   error = .false.
		   j = bv_nifti_io(imagename(1:strlen(imagename)),0,
     +			buffer,ndx,ndy,ndz,out_slice,dz,flip,ip,is)
		   if (j .ne. 0) error = .true.
		else 
			Call read_image(iname,buffer,error)
		endif
		If (error) Then
			write(*, *)"Null image substituted for:",
     +				iname(1:strlen(iname))
		Endif
C
C	flip the image over the X axis
C
		Call flipbyteimage(buffer)
C
C	copy the image into shared memory
C
		Call vt_write_realimage(I,buffer)
	Enddo
C
C	now interpolate the images
C
	start = 0
	isize = (imagesize*imagesize)
	stepsize = (interpimages+1)*isize
C
	write(*, *)"Interpolating images... "
C
	Do i=1,realimages-1
C
C	interpolate the images
C
		iend = start + stepsize
		dest = start + isize
C
C  vl_interp_images_(char *start, char *end, int x,int y,int num,char *dest)
C
		Call vl_interp_images(%val(images_addr+start),
     +				      %val(images_addr+iend),
     +				      %val(imagesize),%val(imagesize),
     +				      %val(interpimages),
     +				      %val(images_addr+dest))
C
C	bump start along
C
		start = start + stepsize
C
	Enddo
C
	error =.false. 
C
	Return
	End
C
C  This routine loads a set of images into texture memory
C  and computes the interpolated images (if requested)
C
C  Uses the following from /globals/:
C	realimages - number of images
C	interslice,interpixel - obvious
C	which - 0=imagevol  1=texturevol
C	flags(1) - number of high-order bits to load and autoscaling mode
C
	Subroutine volume_loadimages(template,interp,error,window,which,
     +		flags)

	Implicit None

	Include 'vt_inc.f'

	byte		buffer(512*512),buf2(512*512)
	logical		error
	Character*200	tname,iname,sname,dirname,roi
	Integer*4	i,start,iend,isize,stepsize,dest,j
	Character*(*)	template
	Integer*4	interp,window,out_slice,which,addr,bits
	Integer*4	flags(*)
	Integer*4	inhist(256),outhist(256),strlen
	External	strlen
	Integer*4	bv_nifti_io
	External bv_nifti_io
	Integer*4	ndx,ndy,ndz,slice,dz,flip,isnifti
	Real*4		is,ip
C
C	NIFTI stuff
C
	isnifti = bv_nifti_io(template(1:strlen(template)),2,0,
     +			ndx,ndy,ndz,slice,dz,flip,ip,is)
C
C	select volume
C
	If (which .eq. 0) Then
		addr = images_addr
	Else
		addr = texture_addr
	Endif
C
C	get the number of bits
C
	bits = iand(flags(1),255)
C
C	Compute the squeeze factor for this dataset
C
        squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)
C
C	Defaults (2=inmin,3=inmax,4=outmin,5=outmax)
C
	flags(3) = 255
	flags(5) = 255
	flags(2) = 0
	flags(4) = 0
C
C	compute mins/maxs if requested
C	
	If (iand(flags(1),256) .eq. 256) Then
C
C	get the min and max of the volume we are about to load
C	and the min and max of the volume in memory right now
C
		Do i=1,256
			inhist(i) = 0
			outhist(i) = 0
		Enddo
C
		If (window .gt. 0) Then
			Call draw_status_bar(float(0)/float(realimages),
     +				"Computing min/max values...}")
		Endif
C
		Do i=1,realimages
C
			tname = " "
			Call get_slice_num(i,out_slice)
			Call name_changer(template,out_slice,iname,error)
			If (error) Return
C
			Do j=1,(imagesize*imagesize)
				buffer(j) = 0
			Enddo
			If (window .gt. 0) Then
			   Call draw_status_bar(float(i)/float(realimages),
     +				"Computing min/max values...}")
			Endif
			if (isnifti .eq. 0) then
			   error = .false.
			   j = bv_nifti_io(template(1:strlen(template)),0,
     +				buffer,ndx,ndy,ndz,out_slice,dz,flip,ip,is)
			   if (j .ne. 0) error = .true.
			else 
				Call read_image(iname,buffer,error)
			endif
			If (.not. error) Then
C
C	here we go...
C
				Call data_histo(buffer,
     +					imagesize*imagesize,inhist)
			Endif
C
			Call vt_image_io_sub(IPC_READ,I,buffer,%val(addr))
			Call data_histo(buffer,
     +				imagesize*imagesize,outhist)
C
		Enddo
C
C	find min/maxes
C
		Do i=1,256
			if (inhist(i) .gt. 0) flags(3) = i-1
			if (outhist(i) .gt. 0) flags(5) = i-1
		Enddo
		Do i=256,2,-1
			if (inhist(i) .gt. 0) flags(2) = i-1
			if (outhist(i) .gt. 0) flags(4) = i-1
		Enddo
C
C	preserve the "zero" case
C
		flags(2) = flags(2)-1
		flags(4) = flags(4)-1
		write(*, *)"Maxs=",flags(3),flags(5)," Mins=",flags(2),flags(4)
C
	Endif
C
C	display...
C
	If (window .gt. 0) Then
		call draw_status_bar(float(0)/float(realimages),
     +			"Loading images...}")
	Endif
C
C	try all the images
C
	Do i=1,realimages
C
C	get the image name
C
		tname = " "
		Call get_slice_num(i,out_slice)
		Call name_changer(template,out_slice,iname,error)
		If (error) Return
C
		Do j=1,(imagesize*imagesize)
			buffer(j) = 0
		Enddo
		If (window .gt. 0) Then
			call draw_status_bar(float(i)/float(realimages),
     +				"Loading images...}")
		Endif
		if (isnifti .eq. 0) then
		   write(*, *)"Reading:",
     +			template(1:strlen(template)),out_slice
		   error = .false.
		   j = bv_nifti_io(template(1:strlen(template)),0,
     +			buffer,ndx,ndy,ndz,out_slice,dz,flip,ip,is)
		   if (j .ne. 0) error = .true.
		else 
			write(*, *)"Reading image:",iname(1:strlen(iname))
			Call read_image(iname,buffer,error)
		endif
		If (error) Then
			write(*, *)"Null image substituted for:",
     +				iname(1:strlen(iname))
		Endif
C
C	flip the image over the X axis
C
		Call flipbyteimage(buffer)
C
C	copy the image into shared memory (directly)
C
		If ((bits .ge. 1).and.(bits .le. 7)) Then
C
C	bit sliced loading...
C
			Call vt_image_io_sub(IPC_READ,I,buf2,%val(addr))
			Call bitslice(buffer,buf2,imagesize*imagesize,
     +				flags(2),flags(3),flags(4),flags(5),bits)
			Call vt_image_io_sub(IPC_WRITE,I,buf2,%val(addr))
		Else
C
C	normal (direct) loading...
C	
			Call vt_image_io_sub(IPC_WRITE,I,buffer,%val(addr))
		Endif
C
	Enddo
C
C	now interpolate the images
C
	start = 0
	isize = (imagesize*imagesize)
	stepsize = (interpimages+1)*isize
C
	write(*, *)"Interpolating images..."
C
	Do i=1,realimages-1
C
		If (window .gt. 0) Then
			call draw_status_bar(float(i)/float(realimages-1),
     +				"Interpolating images...}")
		Endif
C
C	interpolate the images
C
		iend = start + stepsize
		dest = start + isize
C
C  vl_interp_images_(char *start, char *end, int x,int y,int num,char *dest)
C
		If (interp .eq. 0) Then 
			Do j=1,interpimages
				Call memcopy(%val(addr+dest),
     +					%val(addr+start),isize)
				dest = dest + isize
			Enddo
		Else
			Call vl_interp_images(%val(addr+start),
     +				      %val(addr+iend),
     +				      %val(imagesize),%val(imagesize),
     +				      %val(interpimages),
     +				      %val(addr+dest))
		Endif
C
C	bump start along
C
		start = start + stepsize
C
	Enddo
C
	error =.false. 
C
	Return

	End
C
C  This routine saves a set of images from texture memory to disk
C
C  Uses the following from /globals/:
C	realimages - number of images
C	interslice,interpixel - obvious
C
	Subroutine volume_saveimages(template,error,window,which)

	Implicit None

	Include 'vt_inc.f'

	logical		error
	byte		buffer(512*512)
	Character*100	tname,iname
	Integer*4	i,out_slice
	Character*(*)	template
	Integer*4	window,which,addr,strlen,j
	External	strlen
	Integer*4	bv_nifti_io
	External bv_nifti_io
	Integer*4	ndx,ndy,ndz,slice,dz,flip,isnifti
	Real*4		is,ip
C
C	NIFTI stuff
C
	ndx = imagesize
	ndy = imagesize
	ndz = realimages
	dz = volumedepth
	is = interslice
	ip = interpixel
	flip = 0
	isnifti = bv_nifti_io(template(1:strlen(template)),3,0,
     +			ndx,ndy,ndz,slice,dz,flip,ip,is)
C
C	select volume
C
	If (which .eq. 0) Then
		addr = images_addr
	Else
		addr = texture_addr
	Endif
C
C	display...
C
	If (window .gt. 0) Then
		call draw_status_bar(float(0)/float(realimages),
     +			"Saving images...}")
	Endif
C
C	try all the images
C
	Do i=1,realimages
C
C	get the image name
C
		tname = " "
		Call get_slice_num(i,out_slice)
		Call name_changer(template,out_slice,iname,error)
		If (error) Return
C
C	copy the image from shared memory (directly)
C
		Call vt_image_io_sub(IPC_READ,I,buffer,%val(addr))
C
C	flip the image over the X axis
C
		Call flipbyteimage(buffer)
C
		If (window .gt. 0) Then
			call draw_status_bar(float(i)/float(realimages),
     +				"Saving images...}")
		Endif
		if (isnifti .eq. 0) then
		   write(*, *)"Writing:",
     +			template(1:strlen(template)),out_slice
		   error = .false.
		   j = bv_nifti_io(template(1:strlen(template)),1,
     +			buffer,ndx,ndy,ndz,out_slice,dz,flip,ip,is)
		   if (j .ne. 0) error = .true.
		else 
			write(*, *)"Writing image:",iname(1:strlen(iname))
			Call write_image(iname,buffer,error)
		endif
		If (error) Then
			write(*, *)"Unable to write image file:",
     +				iname(1:strlen(iname))
		Endif
C
	Enddo
C
	error =.false. 
C
	Return
	End
C
C       Routine to clear memory to zero
C
        Subroutine      memclear(array,nbytes)

        Implicit None

        Character       array(*)
        Integer*4       nbytes
        Integer*4       i

        Do i=1,nbytes
                array(i) = char(0)
        Enddo

        Return
        End
C
C       Routine to copy memory 
C
        Subroutine      memcopy(dest,src,nbytes)

        Implicit        None

        Character       dest(*),src(*)
        Integer*4       nbytes
        Integer*4       i

        Do i=1,nbytes
                dest(i) = src(i)
        Enddo

        Return
        End
