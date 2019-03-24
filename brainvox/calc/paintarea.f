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
C    MODULE NAME    : paintarea
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
	Subroutine paintarea(trace,num,val,image)          

	Implicit None

	Integer*4	trace(4096,2)
	Integer*4	num,val
	byte		image(512,512)

	Integer*4	list(5000)
	Integer*4	ptr,x,y,i
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
C	paint the points in
C
	Do i=1,ptr-2,3
		Y=list(i+2)
		Do X=list(i),list(i+1)
			image(x,y)=val
		Enddo
	Enddo
C	
	Return
	End
