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
C    MODULE NAME    : bumpsliders
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
C    DESCRIPTION    :  	this routine will allow the user to change
C			the value of the sliders using the keypad
C			parses PAD devices and changes sliders
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call   bumpsliders(dev,state)
C    INPUTS         :          dev : PAD device to handle
C				state : current program state
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine   bumpsliders(dev,state,inc1,inc2)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	integer*4	dev,state,I,J,inc1,inc2
C
C	get the sliders..
C
	If (state .eq. 1) then
		i=state1(3).aux
		J=state1(4).aux
	else
		i=state2(3).aux
		J=state2(4).aux
	endif
C
C	addon based on device...
C
	If (dev .eq. PAD2) then
		j=j+inc1
	Else if (dev .eq. PAD8) then
		j=j-inc1
	Else if (dev .eq. PAD4) then
		i=i-inc1
	Else if (dev .eq. PAD6) then
		i=i+inc1
	Else if (dev .eq. MOUSE2) then
		i = i + inc1
		j = j + inc2
	Endif
C
C block off scrolls
C
	I=min(I,511)
	J=min(J,511)
	I=max(I,0)
	J=max(J,0)
C
C	set the values and return
C
	If (state .eq. 1) then
		state1(3).aux=i
		state1(4).aux=j
	else
		state2(3).aux=i
		state2(4).aux=j
	endif
C	
	Return
	End
