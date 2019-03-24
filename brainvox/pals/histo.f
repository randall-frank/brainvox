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
C    MODULE NAME    : histo
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_pals
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 27 Aug 91       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to compute the histogram of a data volume
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
	Subroutine histogram(hist,csum,volume)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  

	Integer*4	i,hist(256),tempi,csum,volume
C
C	init the buckets
C
	Do i=1,256
		hist(i) = 0
	Enddo
C
C	for each plane
C
        Do i = 1, realimages
C
C       get number of virtual image
c
                tempi = (((i-1)*interpimages)+i)-1
                Call do_plane(hist,
     +                  %val(volume+(tempi*imagesize*imagesize)),
     +                  imagesize)
        Enddo
C
C	checksum the pixels
C
	csum = 0
	Do i=1,256
		csum = csum + (i*hist(i))
	Enddo
C
	Return
	End
C
C	per image subroutine (only REAL images)
C
	subroutine do_plane(hist,image,size)

	Implicit None

	Integer*4	size
	Integer*4	hist(256),i,j

	byte		image(*)

	Do i=1,size*size
		j = image(i)
		if (j .lt. 0) j = j + 256
		hist(j+1) = hist(j+1) + 1
	Enddo

	Return
	End
