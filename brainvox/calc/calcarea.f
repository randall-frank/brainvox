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
	Subroutine calcarea(trace,num,area)      

	Implicit None

	Integer*4	trace(4096,2)
	Integer*4	num,I
	Real*4		area

	Integer*4	list(5000)
	Integer*4	ptr,x,y
C	
C	start at 0.0
C
	area =0.0
C
C	should we be here?
C
	if (num .lt. 3) return
C
C	first find the inside
C
	ptr = 5000
	call polyfill(trace,num,list,ptr)
	if (ptr .lt. 0) return
C
C	count the points in
C
	Do i=1,ptr-2,3
		Y=list(i+2)
		Do X=list(i),list(i+1)
			area = area + 1
		Enddo
	Enddo

C#ifdef OLD_AREA
C
C	loop through the points
C
C	if (num .lt. 3) return
C
C	Do i=2,num
C		area = area + float(trace(i-1,1)*trace(i,2)) - 
C     +				float(trace(i-1,2)*trace(i,1))  
C	Enddo
C
C	closing matrix
C
C	area = area + float(trace(num,1)*trace(1,2)) - 
C     +			float(trace(num,2)*trace(1,1))  
C
C	and absolute value times 1/2
C
C	area = 0.5*abs(area)
C#endif
C	
	Return
	End
