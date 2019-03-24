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
C    MODULE NAME    : dlg_dosbar
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
C    DESCRIPTION    :  	routine handle scroll bar thumbbing...        
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_dosbar(item,sx,sy)
C    INPUTS         :          Item : The SBAR item
C				sx,sy : the initial MOUSE coordinates
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_dosbar(item,sx,sy)

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
	Integer*4	dx,dy,sx,sy,mx,my,oaux,baux
	Integer*4	minmax,min,max
C
	Real*4		fdx,fdy,faux
C
C find max,min
C
	minmax=item.tlength
	min=minmax/65536
	max=iand(minmax,ffff)
C
C	compute dx,dy ==> orientation
C
	dx=item.rect(3)-item.rect(1)
	dy=item.rect(2)-item.rect(4)
	If (dx .gt. dy) Then
		fdy=0.0
		fdx=float(max-min)/float(dx-3*dy)
	Else
		fdx=0.0
		fdy=float(min-max)/float(dy-3*dx)
	Endif
C
C	while the button is down...
C	aux=aux+mousedeltas*scales
C	This is converted to integer but retained in FP for accuracy..
C
	Call dlg_d_sbar(item,5)
	baux=item.aux
	Do while (getbut(guibtn()))
		call dlg_g_mouse(mx,my)
		faux=float(baux)+
     +			((fdx*float(mx-sx))+(fdy*float(my-sy)))
		If (faux .lt. float(min)) faux=float(min)
		If (faux .gt. float(max)) faux=float(max)
		item.aux=jifix(faux)
C
C	redraw only if it moved...
C
		if (item.aux .ne. oaux) Then
			Call dlg_d_sbar(item,5)
			oaux = item.aux
		Endif
	Enddo
C
	Return
	End
