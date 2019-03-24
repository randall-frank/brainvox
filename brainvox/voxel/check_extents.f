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
C    MODULE NAME    : check_extents
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
C    DESCRIPTION    :  	Routine to determine the XY extent of
C			pixel data greater than a threshold
C			It is used to limit the scope of the
C			voxel data set.
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
	Subroutine check_extents(extents,thres,image,size)

	Implicit None

	Integer*4	extents(6),size
	byte		image(*)
	Integer*4	thres
	
	Integer*4	I,J,ptr,k
C
	ptr = 0
	Do J=0,size-1
		Do I=0,size-1
			ptr = ptr + 1
			k = image(ptr)
			if (k .lt. 0) k = k + 256
C
C	is it a bright point???
C
			if (k .gt. thres) then
C
C	X checks
C
				if (i .lt. extents(1)) then
					extents(1) = i
				Endif
				if (i .gt. extents(2)) then
					extents(2) = i
				Endif
C
C	Y checks
C
				if (j .lt. extents(3)) then
					extents(3) =j 
				Endif
				if (j .gt. extents(4)) then
					extents(4) = j
				Endif
			Endif
		Enddo
C
	Enddo
C
	Return
	End
C
C	Routine to update the vset parameters
C
        Subroutine recalc_volume_limits(myset)

        Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'

        Integer*4       myset(*)
        Integer*4       extents(6),thres,i,tempi
C
C       init extents
C
        extents(1) = imagesize-1  ! image Xmin
        extents(2) = 0            ! image Xmax
        extents(3) = imagesize-1  ! image Ymin
        extents(4) = 0            ! image Ymax
        extents(5) = 0            ! image Zmin
        extents(6) = ((realimages-1)*interpimages)+realimages-1 ! image Zmax
C
C       check image data extents
C
        thres = 5
        Do i = 1, realimages
C
C       get number of virtual image
c
                tempi = (((i-1)*interpimages)+i)-1
                Call check_extents(extents,thres,
     +                  %val(images_addr+(tempi*imagesize*imagesize)),
     +                  imagesize)
        Enddo
        Call vl_limits(%val(extents(1)),%val(extents(2)),
     +                 %val(extents(3)),%val(extents(4)),
     +                 %val(extents(5)),%val(extents(6)),myset)

        Return

        End
