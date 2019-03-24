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
C    MODULE NAME    : swapstates
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
C    DESCRIPTION    :  	This routine copies the needed info from 
C			state to state (Namely: DIALOG info)
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call swapstates(state)
C    INPUTS         :          state : old state.             
C    OUTPUTS        : 	       state : new state.
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine swapstates(state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	state
	Character*40	temp
	logical		err
C
	State=3-state
C
C	going to state=1
C
	If (state .eq. 1) then
C
C	copy the scroll bar values...
C
		state1(3).aux=state2(3).aux
		state1(4).aux=state2(4).aux
		state1(5).aux=state2(5).aux
C
C	going to state2
C
	Else
C
C	copy the scroll bar values...
C
		state2(3).aux=state1(3).aux
		state2(4).aux=state1(4).aux
		state2(5).aux=state1(5).aux
C
C	setup text field 16
C
		Call name_changer(rgnnames(cur_roi),cur_slice,temp,err)
		write(state2(17).text,2) temp                          
2	Format('Editing:',A40)
		state2(17).tlength=index(temp,' ')+8 
	Endif
	
	Return
	End
