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
C    MODULE NAME    : dlg_d_edit   
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
C    DESCRIPTION    :  	This routine draws an Edit field
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_edit(item,state)	
C    INPUTS         :          Item : edit item
C				state : active or not..
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  dlg_d_edit(item,state)	

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	record	/ditem/	item
	Integer*4	state	! 0=inactive 1=active
	Integer*4	p1(2),p2(2),dy,temp,fsave

	Integer*4	fmwid
	external	fmwid
C
	temp=item.aux   ! the need for this is due to a bug in f77 
			! which does not allow record fields as
			! arguments to intrinsic functions
C
C	pick font
C
	fsave=cur_font
	cur_font=2
	if (iand(temp,255*256) .ne. 0) then
		cur_font=iand(temp,255*256)/256
                if (cur_font .gt. 10) cur_font=2
	Endif
	Call f_fmsetfont(cur_font)
C
C	fix the rectangle if needed
C
	If (item.rect(3) .eq. -1) then
		item.rect(4)=item.rect(2)-
     +			textheight(cur_font)-8  ! 4 pixel border
		item.rect(3)=item.rect(1)+fmwid(item.text,
     +				item.tlength)+8
	Endif
	dy=item.rect(2)-item.rect(4)
C
C	paint background
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
	If (iand(temp,1) .eq. 1) then
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
C	draw the text...
C
	Call cmov2i(item.rect(1)+4,item.rect(4)+
     +			(dy-textheight(cur_font))/2
     +			+textdecend(cur_font))
	Call fmprstr(item.text(1:item.tlength)//char(0))
C
C	cursor
C
	If (state .eq. 1) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,3))
		Else
			Call color(cbase+3)
		Endif
		p1(1)=item.rect(1)+2+
     +		    jifix(float(fmwid(item.text,item.tpos))*cortopix)
		p2(1)=p1(1)+2
		p1(2)=item.rect(2)-2
		p2(2)=item.rect(4)+2
		Call rectfi(p1(1),p1(2),p2(1),p2(2))
	Endif
C
	cur_font=fsave
	Call f_fmsetfont(cur_font)
C
	Call gfflush
C
	Return
	End
