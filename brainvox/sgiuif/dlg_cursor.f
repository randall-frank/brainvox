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
C    MODULE NAME    : dlg_cursor
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
C    DATE           : 07 Mar 90       
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
	Subroutine dlg_cursor(id)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	id
	Integer*2	cur1(16)

	data	cur1/'1ff0'X,'1ff0'X,'0820'X,'0820'X,
     +		     '0820'X,'0C60'X,'06C0'X,'0100'X,
     +		     '0100'X,'06C0'X,'0C60'X,'0820'X,
     +		     '0820'X,'0820'X,'1FF0'X,'1FF0'X/

	If (id .eq. 0) then
		Call setcur(id,0,0)
	Else if (id .eq. 1) then
		Call defcur(id,cur1)
		Call curori(id,8,8)
		Call setcur(id,0,0)
	Endif

	Return
	End
