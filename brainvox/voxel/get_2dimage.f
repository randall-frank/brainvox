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
C    MODULE NAME    : get_2dimages
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to retrieve a cmap image from shared memory
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
	Subroutine get_2dimage(num,image,ban)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	
	Integer*4	num,i,ban

	Integer*2	image(*)
	byte		tim(512*512)
C
C	get byte image
C
	If (ban .eq. 1) Then
		Call vt_image_io_sub(IPC_READ,num,tim,%val(images_addr))
	Else
		Call vt_image_io_sub(IPC_READ,num,tim,%val(texture_addr))
	Endif
C
C	convert to I*2
C
	Do I=1,imagesize*imagesize
C
C	get byte
C
		image(i) = tim(i)
C
C	convert to i*2
C
		if (image(i) .lt. 0) image(i) = image(i) + 256
C
C	add lut offset
C
		image(i) = image(i) + 256
C
	Enddo
C
	Return
	End
