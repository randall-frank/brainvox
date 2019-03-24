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
C    MODULE NAME    : delpoints
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
C    DESCRIPTION    :  	
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
	Subroutine delpoints(num,list,id)

	Implicit None

	Integer*4	num,id,list(4096,4)
	Integer*4	temp(4096,4),i,j,k

C
C	remove all points from list() with list(*,4) = id
C
C	first copy the array
C
	Do I=1,num
		Do J=1,4
			temp(I,j) = list(i,j)
		Enddo
	Enddo
C
C	next scan through the list copying good points
C
	k=0
	Do i=1,num
		if (temp(i,4) .ne. id) then
			k = k + 1
			Do J=1,4
				list(k,j)=temp(i,j)
			Enddo
		
		Endif
	Enddo
C
C	patch up num and return
C
	num = k
	
	Return
	End
