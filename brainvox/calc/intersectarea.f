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
C    MODULE NAME    : Intersectarea
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_calc
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
	Subroutine intersectarea(trace,num,counts,image)

	Implicit None

	Integer*4	trace(4096,2)
	Integer*4	num,counts(127)
	byte		image(512,512)
C
	Integer*4	i,j
        Integer*4       list(5000)
        Integer*4       ptr,x,y
C
C	init to flag value
C
	Do I=1,127
		counts(I) = -1
	Enddo
C
C       should we be here?
C
        if (num .lt. 3) return
C
C       first find the inside
C
        ptr = 5000
        call polyfill(trace,num,list,ptr)
        if (ptr .lt. 0) return
C
C       look for the points in
C
        Do i=1,ptr-2,3
                Y=list(i+2)
                Do X=list(i),list(i+1)
			J= image(x,y)
C
C	if point is of a lesion
C
                        if (J .gt. 0) then
C
C	if no previous then init the counter
C
				if (counts(J) .eq. -1) counts(J)=0
C
C	we found one!!!
C
				counts(j) = counts(j) + 1
			Endif
                Enddo
        Enddo
C	
	Return
	End
