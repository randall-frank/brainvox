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
C    MODULE NAME    : dlg_d_button
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
C    DESCRIPTION    :  	This routine draws a button
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call  dlg_d_button(item,state)
C    INPUTS         : Item : the button definition
C			state : highlighted or not...
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  dlg_d_button(item,state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  
	
	Integer*4	fmwid
	external	fmwid

	Record	/ditem/	item
	Integer*4	state
C
C	fix the rectangle if needed
C
	If (item.rect(3) .eq. -1) then
		item.rect(4)=item.rect(2)-
     +			textheight(cur_font)-8  ! 4 pixel border
		item.rect(3)=item.rect(1)+fmwid(item.text,
     +				item.tlength)+8
	Endif
C
C	paint  background
C
	If (state .eq. 1) then    !inverted
		If (RGBmode) then
			Call c3i(dlg_colors(1,5))
		Else
			Call color(cbase+5)
		Endif
	Else
		If (RGBmode) then
			Call c3i(dlg_colors(1,4))
		Else
			Call color(cbase+4)
		Endif
	Endif
	Call rectfi(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
C
C	frame
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,1))
	Else
		Call color(cbase+1)
	Endif
	Call recti(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
C
C	text
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,7))
	Else
		Call color(cbase+7)
	Endif
C
	Call dlg_center(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4),item.text,item.tlength)
C
	Call gfflush
C
	Return
	End
