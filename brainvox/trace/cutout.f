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
C    MODULE NAME    : _Main   
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_trace
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine cuts the passed ROI from the image
C			and saves the image in the appropriate dir
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
	Subroutine cutout(trace,side,error)

	Implicit None

	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	
	Real*4		fact
	Integer*4	thintrace(4096,2)
	Integer		I,ii,jj,x,y,side,isnifti,bv_nifti_io
	Integer*4	count,trace(4096,2),list(4096),ptr,out_slice,strlen
	byte		outimg(512*512)  ! max size output image
	byte		inimg(512*512)
	Character*200	filename,temp,stemp,roi,dir
	logical		error
	External	strlen,bv_nifti_io
C
C	did we get here by mistake ???
C
	If (cut_dir  .ne. 0) then
		error = .false.
		return
	Endif
C
C	if the original image depth is > 1 byte/pixel, no go...
C
	If (volumedepth .ne. 1) then
		Call bv_error("Brainvox cannot cut input images greater|"
     +			//"than 8bits deep.}",0)
		return
	Endif
C
C	Get the directory name
C
	Call indexnum_to_roi(side,roi,i)
	If (i .lt. 0) then
		write(*, *)"Incorrect SIDE variable"
		error = .false.
		return
	Endif
	Call roi_to_dir(roi,dir)
C
C	zero out destination image
C
	Do I=1,imagesize*imagesize
		outimg(i) = 0
	Enddo
C
C	check for badtrace
C
	if (trace(1,1) .lt. 3) then
		error = .true.
		write(*, 14)
14	Format('Unable to complete roi fill operation. Roi too simple.')
		return
	Endif
C
C	get reduction factor
C
	fact = 512.0/float(imagesize)
C
C	get the real image
C
	call vt_read_realimage(cur_slice,inimg)
C
C	convert the input trace format to the polyfill trace format
C	Note: the ROI must be shrunk by FACT to go from 512x512 to imagesize
C
	count = 0          
	Do i=1,trace(1,1)
		X = (float(trace(i+1,1))/fact)
		Y = (float(trace(i+1,2))/fact)
		if (X .lt. 0) X = 0
		If (Y .lt. 0) Y = 0
		if (X .gt. imagesize-1) X = imagesize - 1
		If (Y .gt. imagesize-1) Y = imagesize - 1
C
C	thin out replicants
C
		if (count .eq. 0) then         ! always the first one
			count = count + 1
			thintrace(count,1) = X
			thintrace(count,2) = Y
		Else if (X .ne. thintrace(count,1)) then ! add if different
			count = count + 1
			thintrace(count,1) = X
			thintrace(count,2) = Y
		Else if (Y .ne. thintrace(count,2)) then ! add if different
			count = count + 1
			thintrace(count,1) = X
			thintrace(count,2) = Y
		Endif
C
	Enddo
C
C	call polyfill
C
	ptr=4096
	Call polyfill(thintrace,count,list,ptr)
	if (ptr .eq. -1) then
		error = .true.
		write(*, 13)
13	Format('Unable to complete roi fill operation. Roi too complex.')
		return
	Endif
C
C do for ROI region  (copy pixels from inside to outside...)
C	for each line
C
	Do i=1,ptr-2,3
C
C	and each pixel  (note: images from shared mem have been flipped)
C
		Y=list(i+2)
		Do X=list(i),list(i+1)
C
C	get the index and add to the count
C
			jj=y*imagesize+x
C 
C	copy pixels from in image into out image (+1 for Fortran offset)
C
			outimg(jj+1)=inimg(jj+1)
		Enddo
	Enddo
C
C	flip the output image over the X axis
C
	Call flipbyteimage(outimg)
C
C	get the filename
C
	Call get_slice_num(cur_slice,out_slice)
	if (bv_nifti_io(imagename(1:strlen(imagename)),3,0,
     +		imagesize,imagesize,realimages,out_slice,volumedepth,0,
     +		interpixel,interslice) .eq. 0) Then
		temp = imagename(1:strlen(imagename))
		isnifti = 0
	Else
		isnifti = -1
		temp = " "
		call name_changer(imagename,out_slice,temp,error)
		if (error) return
	Endif
C
C	get the simple part of the imagename (remove leading dirs...)
C
	i = 200
	Do while ((i .gt. 0) .and. (temp(i:i) .ne. '/'))
		i = i - 1
	Enddo
	stemp = temp(i+1:200)
C
C	subdirectory added to the filename
C
	filename=dir(1:strlen(dir))//"/"//stemp(1:strlen(stemp))
C
C	write the file 
C
	if (isnifti .eq. 0) then
		error = .false.
		if (bv_nifti_io(filename(1:strlen(filename)),1,outimg,
     +			imagesize,imagesize,realimages,out_slice,
     +			volumedepth,0,interpixel,interslice) .ne. 0) then
			error = .true.
		endif
	else
		Call write_image(filename,outimg,error)
	endif
	If (error) Then
		Call bv_error("Unable to save cut image to disk|"//
     +			"Check disk space and permissions.}",0)
	Endif
C
C	done
C
	Return
	End
