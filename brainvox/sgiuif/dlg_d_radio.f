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
C    MODULE NAME    : dlg_d_radio    
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
C    DESCRIPTION    :  	Draws a Radio button...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_radio(item,state)
C    INPUTS         :          Item : checkbox def
C				state : Highlight state
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_d_radio(item,state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Record	/ditem/	item
	Integer*4	dy,p1(2),state

	integer*4	fmwid
	external	fmwid

	Integer*4	inset
	parameter	(inset=3)
C
C	state=1=feedback  0=inactive
C
C	fix the rectangle if needed
C
	If (item.rect(3) .eq. -1) then
		item.rect(4)=item.rect(2)-
     +				textheight(cur_font)-8  ! 4 pixel border
		item.rect(3)=item.rect(1)+fmwid(item.text,
     +				item.tlength)+8
		item.rect(3)=item.rect(3)+
     +				textheight(cur_font)+8 !space for the box  
	Endif
C
C	find dy
C
	dy=item.rect(2)-item.rect(4)
C
C	paint  background
C
	If (state .eq. 0) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,4))
		Else
			Call color(cbase+4)
		Endif
	Else
		If (RGBmode) then
			Call c3i(dlg_colors(1,5))
		Else
			Call color(cbase+5)
		Endif
	Endif
	Call rectfi(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
C
C	find the center of the circle (right hand side with inset)
C
	p1(1)=item.rect(3)-inset-(dy/2)
	p1(2)=item.rect(4)+(dy/2)
C
C	draw the box
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,1))
	Else
		Call color(cbase+1)
	Endif
	Call circi(p1(1),p1(2),(dy/2)-inset)
C
C	draw the center
C
	If (item.aux .eq. 1) then
		Call circfi(p1(1),p1(2),(dy/2)-inset-3)
	Endif
C
C	text
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,7))
	Else
		Call color(cbase+7)
	Endif
C
	Call dlg_center(item.rect(1),item.rect(2),
     +		item.rect(3)-8-textheight(cur_font),
     +		item.rect(4),item.text,item.tlength)
C
	Call gfflush
C
	Return
	End
