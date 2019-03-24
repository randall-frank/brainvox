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
C    MODULE NAME    : draw_grid
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 24 Jan 91       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine draw_grid(zoom,mcolor,cm)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  

	Integer*4	dx,dy,zoom,mcolor

	real*4		p1(2),p2(2),fzoom,cm

	Character*15	tstr
C
C	get window extents
C
	Call getsiz(dx,dy)
	fzoom = float(zoom)
C
C	set color (based on window mode)
C
	Call bv_set_color(mcolor)
C
C	horiz grid lines
C
	p1(1) = 0
	p2(1) = float(dx)
	p1(2) = 0
	Do While (p1(2) .le. dy)
		p2(2) = p1(2)
		call bgnlin
		call v2f(p1)
		call v2f(p2)
		call endlin
		p1(2) = p1(2) + ((cm*10.0*fzoom)/interpixel)
	Enddo
C
C	vert grid lines
C
	p1(2) = 0
	p2(2) = float(dy)
	p1(1) = 0
	Do While (p1(1) .le. dx)
		p2(1) = p1(1)
		call bgnlin
		call v2f(p1)
		call v2f(p2)
		call endlin
		p1(1) = p1(1) + ((cm*10.0*fzoom)/interpixel)
	Enddo
C
C	text
C
	Call cmov2i(10,10)
	write(unit=tstr,fmt=10) cm
10	Format("Sq=",F5.2,"cm")
	Call charst(tstr,10)
C
	Return
	End
