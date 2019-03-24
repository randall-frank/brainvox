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
C    MODULE NAME    : dlg_frects
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
C    DESCRIPTION    :  	routine to find the rectangles of a scroll bar
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call       dlg_frects(item,rects)
C    INPUTS         :          Item : SBAR def
C    OUTPUTS        : 
C				rects : Resultant rectangles
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_frects(item,rects)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  
	
	Integer*4	ffff
	parameter	(ffff = 65535)

	Record	/ditem/	item
	Integer*4	rects(4,5)
C
	Integer*4	dx,dy
	Logical		horiz
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
	center=float(item.aux-(minmax/65536))/
     +		float(iand(minmax,ffff)-(minmax/65536))
	If (center .lt. 0.0) center=0.0
	If (center .gt. 1.0) center=1.0
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
C	compute the 6 points defining a scroll bar:
C
C      1     6      3
C       <----[ ]----->    for the vertical flip vertically & rot 90 righthand
C        2     5      4
C
C	this makes 5 rectangles (1-2) (2^-6') (6-5) (5^-3') (3-4)
C       
	If (horiz) then 
C
C	horizontal
C
		Rects(1,1)= x1
		Rects(2,1)= y1
		Rects(3,1)= x1+dy
		Rects(4,1)= y2
C
		Rects(1,2)= x2-dy
		Rects(2,2)= y1
		Rects(3,2)= x2
		Rects(4,2)= y2
C
		Rects(1,3)= x1+dy
		Rects(2,3)= y1
		Rects(3,3)= x1+jifix(center*float(dx-dy*3))+dy
		Rects(4,3)= y2
C
		Rects(1,4)= rects(3,3)+dy                             
		Rects(2,4)= y1
		Rects(3,4)= x2-dy
		Rects(4,4)= y2
C
		Rects(1,5)= rects(3,3)
		Rects(2,5)= y1
		Rects(3,5)= rects(1,4)
		Rects(4,5)= y2
C
	Else
C
C	vertical
C
		Rects(1,1)= x1
		Rects(2,1)= y1
		Rects(3,1)= x2
		Rects(4,1)= y1-dx
C
		Rects(1,2)= x1
		Rects(2,2)= y2+dx
		Rects(3,2)= x2
		Rects(4,2)= y2
C
		Rects(1,3)= x1
		Rects(2,3)= y1-dx
		Rects(3,3)= x2
		Rects(4,3)= y1-jifix(center*float(dy-3*dx))-dx
C
		Rects(1,4)= x1
		Rects(2,4)= Rects(4,3)-dx
		Rects(3,4)= x2
		Rects(4,4)= y2+dx
C
		Rects(1,5)= x1         
		Rects(2,5)= Rects(4,3)
		Rects(3,5)= x2           
		Rects(4,5)= Rects(2,4)
C
	Endif
C
	Return
	End
