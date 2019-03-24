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
C    MODULE NAME    : dlg_d_sbar
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
C    DESCRIPTION    :  	Routine to draw a scroll bar
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_sbar(item,state)
C    INPUTS         :          Item: SBAR defs 
C				state : active portion number
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_d_sbar(item,state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  
	
	Integer*4	ffff
	parameter	(ffff = 65535)

	Record	/ditem/	item
	Integer*4	state   !0=dormant 1,2=line u/d 3,4=page u/d 5=thumb
	Integer*4	dx,dy
	Logical		horiz
	Integer*4	p1(2),p2(2),p3(2),p4(2),p5(2),p6(2),p7(2),p8(2)
	Integer*4	p9(2),p10(2)
	Real*4		center
	Integer*4	x1,y1,x2,y2
	Integer*4	minmax
C
C
	x1=item.rect(1)
	y1=item.rect(2)
	x2=item.rect(3)
	y2=item.rect(4)
C
C	compute center=(aux-min)/(max-min)  
C
	minmax=item.tlength
	If (iand(minmax,ffff) .eq. (minmax/65536)) then
		center = 0.
	Else
		center=float(item.aux-(minmax/65536))/
     +			float(iand(minmax,ffff)-(minmax/65536))
	Endif
	If (center .lt. 0.) center=0.
	If (center .gt. 1.) center=1.
C
C	compute dx,dy ==> orientation
C
	dx=item.rect(3)-item.rect(1)
	dy=item.rect(2)-item.rect(4)
	If (dx .gt. dy) Then
		horiz=.true.
	Else
		horiz=.false.
	Endif
C
C	compute the 10 points defining a scroll bar:
C
C      1 9   6      3
C     7 <----[ ]-----> 8  for the vertical flip vertically & rot 90 righthand
C        2     5   10 4
C       
	If (horiz) then 
C
C	horizontal
C
		p1(1)=x1          
		p1(2)=y1
C
		p2(1)=x1+dy
		p2(2)=y2         
C
		p3(1)=x2-dy
		p3(2)=y1
C
		p4(1)=x2
		p4(2)=y2
C
		p5(1)=x1+jifix(center*float(dx-3*dy))+2*dy
		p5(2)=y2
C
		p6(1)=p5(1)-dy
		p6(2)=y1
C
		p7(1)=x1
		p7(2)=y1-dy/2
C
		p8(1)=x2
		p8(2)=p7(2)
C
		p9(1)=x1+dy
		p9(2)=y1
C
		p10(1)=x2-dy
		p10(2)=y2
C
	Else
C
C	vertical
C
		p1(1)=x1          
		p1(2)=y1
C
		p2(1)=x2   
		p2(2)=y1-dx      
C
		p3(1)=x1    
		p3(2)=y2+dx
C
		p4(1)=x2
		p4(2)=y2
C
		p5(1)=x2
		p5(2)=y1-jifix(center*float(dy-3*dx))-2*dx
C
		p6(1)=x1
		p6(2)=p5(2)+dx
C
		p7(1)=x1+dx/2
		p7(2)=y1
C
		p8(1)=p7(1)
		p8(2)=y2
C
		p9(1)=x1
		p9(2)=y1-dx
C
		p10(1)=x2
		p10(2)=y2+dx
C
	Endif
C
C	fill feedback first...	
C
	If (state .eq. 1) then
C
C	up/left arrow
C
		If (RGBmode) then
			Call c3i(dlg_colors(1,5))
		Else
			Call color(cbase+5)
		Endif
		Call rectfi(p1(1),p1(2),p2(1),p2(2))
	Else
		If (RGBmode) then
			Call c3i(dlg_colors(1,4))
		Else
			Call color(cbase+4)
		Endif
		If (state .eq. 0)Call rectfi(p1(1),p1(2),p2(1),p2(2))
	Endif
C
	If (state .eq. 2) then
C
C	down/right arrow
C
		If (RGBmode) then
			Call c3i(dlg_colors(1,5))
		Else
			Call color(cbase+5)
		Endif
		Call rectfi(p3(1),p3(2),p4(1),p4(2))
	Else
		If (RGBmode) then
			Call c3i(dlg_colors(1,4))
		Else
			Call color(cbase+4)
		Endif
		If (state .eq. 0) Call rectfi(p3(1),p3(2),p4(1),p4(2))
	Endif
C
C	fill the thumb
C
	If (state .eq. 5) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,5))
		Else
			Call color(cbase+5)
		Endif
		Call rectfi(p5(1),p5(2),p6(1),p6(2))
	Else
		If (RGBmode) then
			Call c3i(dlg_colors(1,4))
		Else
			Call color(cbase+4)
		Endif
		If (state .eq. 0) Call rectfi(p5(1),p5(2),p6(1),p6(2))
	Endif
C
C	fill pagers
C
C	paging sections
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,6))
	Else
		Call color(cbase+6)
	Endif
	If ((state .eq. 0).or.(state .eq. 5)) then
		Call rectfi(p2(1),p2(2),p6(1),p6(2))
		Call rectfi(p3(1),p3(2),p5(1),p5(2))
	Endif
C
C	arrow lines
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,1))
	Else
		Call color(cbase+1)
	Endif
	Call bgnclo
	Call v2i(p9)
	Call v2i(p7)
	Call v2i(p2)
	Call endclo
	Call bgnclo
	Call v2i(p3)
	Call v2i(p8)
	Call v2i(p10)
	Call endclo
C
C	frame thumb
C	
	Call recti(p5(1),p5(2),p6(1),p6(2))
C
C	frame whole...
C
	If (RGBmode) then
		Call c3i(dlg_colors(1,1))
	Else
		Call color(cbase+1)
	Endif
	Call recti(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
C	
	Call gfflush
C
	Return
	End
