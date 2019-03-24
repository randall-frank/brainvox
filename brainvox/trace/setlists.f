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
C    MODULE NAME    : setlists
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
C    DESCRIPTION    :  	This routine sets the scrolling lists TEXT to
C			match the input settings...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call setlists(back,roi)
C    INPUTS         :          back : name at the top of the background list
C				roi : Name at the top of the roinames list
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine setlists(back,roi)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	Include 'vt_arb_inc.f'
	
	Integer*4	back,roi,I,J
	Character*24	junk
C
C	background roinames...
C
	J=21
	Do I=back,back+7
		if (i .le. numrgns) then
			state1(J).tlength=24    
			state2(J).tlength=24    
			If (background(I)) then
				state1(J).aux=1
				state2(J).aux=1
			else
				state1(J).aux=0
				state2(J).aux=0
			endif
			junk = rgnnames(I)(1:24)
			state1(J).text=junk
			state2(J).text=junk
		Else
			state1(j).tlength= 1
			state2(j).tlength= 1
			state1(J).aux=0
			state2(J).aux=0
			state1(j).text=" "
			state2(j).text=" "
		Endif
		j=j+1
	Enddo
C
C	roi select names...
C
	J=29
	Do I=roi,roi+7   
		If (i .le. numrgns) then
			junk=rgnnames(I)(1:24)
			state1(J).tlength=24    
			state1(J).aux=0
			state1(J).text=junk
			state1(J).tpos=1
			If ((i .eq. 1) .and. (arb_mode .eq. 1)) Then
				state1(J).aux=DLG_M_GRAY
				state1(J).tpos=0
			Endif
			If (arb_mode .gt. 1) Then
				If (cur_settings(arb_mode).name .ne.
     +					rgnnames(i)) Then

					state1(J).aux=DLG_M_GRAY
					state1(J).tpos=0
				Endif
			Endif
		Else
			state1(J).tlength=1     
			state1(J).tpos=0
			state1(J).aux=0
			state1(J).text=" "         
		Endif
		j=j+1
	Enddo
C	
	Return
	End
