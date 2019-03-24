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
C    MODULE NAME    : tr_read_images
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
C    DESCRIPTION    :  	This routine reads images from dynamic memory
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call tr_read_images(num,I1,I2)
C    INPUTS         :          Num : image number
C				(roots are unused..)
C    OUTPUTS        : I1,I2 images that were read......
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_read_images(num,I1,I2)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'
	Include 'vt_arb_inc.f'
	
	Integer*4	num
	Character*200	filename
	Integer*4	I,id,ii,kk,k
	Integer*4	x,y,ptr,mult
C
	Integer*2	I1(512*512)
	Integer*2	I2(512*512)
	Byte		image(512*512)
	Byte		timage(512*512)
C
	Real*4		rots(3),dist
C
C	Get the PD and T2 images
C
	If (arb_mode .ne. 0) Then

		Call slice_to_helddata(cur_slice)
C
C	get rots
C
		rots(1) = cur_settings(arb_mode).rots(1)
		rots(2) = cur_settings(arb_mode).rots(2)
		rots(3) = cur_settings(arb_mode).rots(3)
C
C	Calc dist = Base + (slice-1)*is/ip
C
		dist = cur_settings(arb_mode).base_slice + 
     +			((cur_slice-1)*cur_settings(arb_mode).islice)
     +			/interpixel
C
C	Sample the images from shared memory
C
		Call vl_resample(%val(imagesize),%val(imagesize),I1,
     +			rots,dist,myset)
		Call vl_resample16(%val(imagesize),%val(imagesize),I2,
     +			rots,dist,myset)
C
C	Convert to 8bits
C
		Do i=1,imagesize*imagesize
			image(i) = I1(i) - 256
			timage(i) = I2(i) - 256
		Enddo
C
	Else
C
C	Call the dynamic memory routine to retrieve slices
C
		Call vt_read_realimage(num,image) ! use shared memory
		Call vt_image_io_sub(IPC_READ,num,timage,%val(texture_addr))
	Endif
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
				I1(ptr+k-1) = ii
			Enddo  ! next pixel repeat
C
			ii = timage(x + (y*imagesize))
			if (ii .lt. 0) ii = 256 + ii
			ii = ii + 256
C
C	repeat pixels along the scan line
C
			Do k = 1, mult
				I2(ptr) = ii
				ptr = ptr + 1
			Enddo  ! next pixel repeat

		    Enddo   ! next pixel
		  Enddo  ! next scan line repeat
	Enddo  ! next scan line  
C
	Return
C
	End
C

	Subroutine slice_to_helddata(slice)
	
	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'
	Include 'vt_arb_inc.f'
C
	Integer*4	slice
C
	Real*4		rots(3),dist
C
C       get rots
C
        rots(1) = cur_settings(arb_mode).rots(1)
        rots(2) = cur_settings(arb_mode).rots(2)
        rots(3) = cur_settings(arb_mode).rots(3)
C
C       Calc dist = Base + (slice-1)*is/ip
C
        dist = cur_settings(arb_mode).base_slice + 
     +                  ((slice-1)*cur_settings(arb_mode).islice)
     +                  /interpixel
C
C       setup internal vectors
C
        Call vl_sample_vectors(%val(imagesize),%val(imagesize),
     +          rots,dist,myset)

	Return
	End
