c    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C	Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : bv_error
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 3 Feb 95      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	this file contains routines for presenting the user
C			with a possibly model dialog containing an error msg.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C
C    LIMITATIONS    :      
C    NOTES          :    See individual routines
C                 
C    ====================================================================== 
C
	Subroutine bv_error(text,modal)

	Implicit None

	Include '../libs/vt_inc.f'
C
	Character*(*)	text
	Integer*4	modal
	Character*1024	prog
C
	If (index(text,"}") .eq. 0) Then
		write(*, *)"Malformed Brainvox error message:",text
		Return
	Endif
C
	If (modal .eq. 0) Then
		Write(unit=prog,fmt=10) shm_data,0,
     +			text(1:index(text,"}")-1),"&"
	Else
		Write(unit=prog,fmt=10) shm_data,0,
     +			text(1:index(text,"}")-1)," "
	Endif
10	Format("brainvox_filer ",I," ",I,' "',A,'" "Ok" -c -b ',A1)
C
	Call bv_system_path(prog)

        Return
        End   
