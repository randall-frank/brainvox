
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
C    MODULE NAME    : calcarea
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
C    DESCRIPTION    :  	computes the area of an ROI (in pixels)
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     call calcarea(roi,area)
C    INPUTS         :          roi(4096,2) (1,1)=count
C    OUTPUTS        : 		area = area in pixels
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine calcarea(roi,area)

	Implicit None

	Integer*4	roi(4096,2)
	Real*4		area

	Integer*4	I,J

	area = 0.0

	if (roi(1,1) .lt. 3) return

	J = roi(1,1)
	Do I=2,J
		area = area + 
     +			float((roi(I,1)*roi(I+1,2))-(roi(I+1,1)*roi(I,2)))
	Enddo
	
	area = area +
     +			float((roi(J,1)*roi(2,2))-(roi(J+1,1)*roi(2,2)))

	area = abs(area)/2.0
	
	Return
	End
