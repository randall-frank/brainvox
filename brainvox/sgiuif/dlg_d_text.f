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
C    MODULE NAME    : dlg_d_text
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
C    DESCRIPTION    :  	This routine draws a text string
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_text(item)
C    INPUTS         :          Item : text item
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_d_text(item)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  
	
	integer*4	fmwid,temp
	external	fmwid

	Integer*4	xr,yr,dy,fsave

	Record	/ditem/	item
C
C	get the aux value in a way we can use it...
C
	temp=item.aux
C
C	change font if needed...
C
	fsave=cur_font
	cur_font=2
	if (iand(temp,DLG_M_FONT) .ne. 0) then
		cur_font=iand(temp,DLG_M_FONT)/256
		if (cur_font .gt. 10) cur_font=2
	Endif
	Call f_fmsetfont(cur_font)
C
C	fix the rectangle if needed
C
	If (item.rect(3) .eq. -1) then
		item.rect(4)=item.rect(2)-
     +				textheight(cur_font)-8  ! 4 pixel border
		item.rect(3)=item.rect(1)+fmwid(item.text,
     +				item.tlength)+8
	Endif
C
C	invert background
C
	If (iand(temp,1) .eq. DLG_M_INVERT) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,3))
		Else
			Call color(cbase+3)
		Endif
		Call rectfi(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
	Endif
C
C	frame
C
	If (iand(temp,2) .eq. DLG_M_FRAME) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,1))
		Else
			Call color(cbase+1)
		Endif
		Call recti(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
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
C	Is the text to be grayed??
C
	If (iand(temp,16) .eq. DLG_M_GRAY) Then
		If (RGBmode) then
			Call c3i(dlg_colors(1,3))
		Else
			Call color(cbase+3)
		Endif
	Endif
C
C	pick justification and draw
C
	dy=item.rect(2)-item.rect(4)
        yr=item.rect(4)+(dy-textheight(cur_font))/2+textdecend(cur_font)
C
C	select between justify modes...
C
	if (iand(temp,DLG_M_JUSTIFY) .eq. DLG_J_CENT) then   !center text
		Call dlg_center(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4),item.text,item.tlength)
	Else if (iand(temp,DLG_M_JUSTIFY) .eq. DLG_J_LEFT) then ! left justify
		xr=item.rect(1)+4
		call cmov2i(xr,yr)
        	Call fmprstr(item.text(1:item.tlength)//char(0))
	Else if (iand(temp,DLG_M_JUSTIFY) .eq. DLG_J_RIGHT) then ! right justify
		xr=item.rect(3)-fmwid(item.text,item.tlength)-4
		call cmov2i(xr,yr)
        	Call fmprstr(item.text(1:item.tlength)//char(0))
	Endif
C
	Call gfflush
C
C	restore font
C
	cur_font=fsave
	Call f_fmsetfont(cur_font)
C		
	Return
	End
