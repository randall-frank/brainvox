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
C    MODULE NAME    : dlg_inrect
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : sgiuif
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 08 Aug 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This function determines if a point is in a rectangle
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C		logical=dlg_inrect(rect,x,y)
C    INPUTS         :          X,Y : point in question
C				rect : rectangle in question
C    OUTPUTS        :  Function returns true or false
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Logical	function dlg_inrect(rect,x,y)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	rect(4)
	Integer*4	x,y
C
C	check x
C
	If ((x .gt. rect(3)).or.(x .lt. rect(1))) then
		dlg_inrect=.false.
		return
	Endif
C
C	check y
C
	If ((y .gt. rect(2)).or.(y .lt. rect(4))) then
		dlg_inrect=.false.
		return
	Endif
C
C	the point is in...
C
	dlg_inrect=.true.
C	
	Return
	End
