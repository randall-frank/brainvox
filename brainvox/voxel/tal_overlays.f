
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
C    MODULE NAME    : Talairach_overlays
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : Brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 14 Dec 92       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to implement Talairach space overlays
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
C
C       Subroutine to setup the ortho params and draw the TALspace
C       graphics into the voxel image window (3d)
C
        Subroutine draw_tal_overlay(myset,imagesize,
     +          which,offset,zoom,mcolor)

        Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'tal_space.inc'

        Integer*4               myset(*),imagesize,mcolor
        Integer*4               which,offset(2),zoom
C
        Integer*4               a,b
        Real*4                  dx,dy
        Real*4                  oth(6)
	Integer*4		points(8*3)
	Real*4			tal(3)
	Integer*4		i
C
	If (which .eq. 0) Return
	If (tspace.valid .eq. 0) Then
		write(*, 1)
1	Format("Talairach space must be calibrated before ",
     +			"it can be drawn.")
		Return
	Endif
C
C	set color index (or color)
C
	Call bv_set_color(mcolor)
C
C       Get windowsize/2
C
        Call getsiz(a,b)
        dx = float(a)/float(zoom)
        dy = float(b)/float(zoom)
        a = 445/2
        b = 445/2
C
C       Generate the ortho parameters
C
        oth(1) = (-a) + offset(1)
        oth(2) = oth(1) + dx
        oth(3) = (-b) + offset(2)
        oth(4) = oth(3) + dy
C
C       Z depths are fixed
C
        oth(5) = -(445/2)
        oth(6) = (445/2)
C
        Call ortho(oth(1),oth(2),oth(3),oth(4),oth(5),oth(6))
C
C	Compute the points 
C
C	XY plane
C
	If (iand(which,1) .eq. 1) Then
C left side section
		i = 1
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C right side section
		i = 1
		tal(1) = TAL_RIGHT
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
c front to 0.0 section
		i = 1
		tal(1) = TAL_RIGHT
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = TAL_FRONT
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C -(m+b) to -(m) section
		i = 1
		tal(1) = TAL_RIGHT
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = -TAL_MIDDLE
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = -TAL_MIDDLE
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C 0.0 to -(m) section
		i = 1
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = -TAL_MIDDLE
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = -TAL_MIDDLE
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,3,0,myset)
	Endif
C
C	XZ plane
C
	If (iand(which,2) .eq. 2) Then
C right side section
		i = 1
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) =  TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C left side section
		i = 1
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) =  TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) =  TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C top to 0.0 section
		i = 1
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,8,0,myset)
C 0.0 to bottom section
		i = 1
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = -TAL_LEFT
		tal(2) = 0.0
		tal(3) =  0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = TAL_RIGHT
		tal(2) = 0.0
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
	Endif
C
C	YZ plane
C
	If (iand(which,4) .eq. 4) Then
C bottom section
		i = 1
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C top section
		i = 1
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = 0.0
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,8,0,myset)
C front 1/3
		i = 1
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = TAL_FRONT
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
C middle 1/3
		i = 1
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = 0.0
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -TAL_MIDDLE
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -TAL_MIDDLE
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,3,0,myset)
C last 1/3
		i = 1
		tal(1) = 0.0
		tal(2) = -TAL_MIDDLE
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -TAL_MIDDLE
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = TAL_TOP
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
		tal(1) = 0.0
		tal(2) = -(TAL_MIDDLE+TAL_BACK)
		tal(3) = -TAL_BOTTOM
		call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		Call vl_draw_tal_plane(points,4,0,myset)
	Endif
C
        Return
        End
C
C	Subroutine to compute the overlay of the tal_space onto a
C	given slice plane (arbitrary mode)
C
	Subroutine draw_tal_arb_overlay(which,offset,zoom,mcolor)

        Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'tal_space.inc'

        Integer*4               mcolor
        Integer*4               which,offset(2),zoom
C
	Integer*4		points(12*3)
	Real*4			tal(3),f,inc
	Integer*4		i,j
C
	If (which .eq. 0) Return
	If (tspace.valid .eq. 0) Then
		write (*, 1)
1	Format("Talairach space must be calibrated before ",
     +			"it can be drawn.")
		Return
	Endif
C
C	set color index (or color)
C
	Call bv_set_color(mcolor)
C
C	Compute the points 
C
C	Start with the planes parallel to YZ (X=0)
C
	f = -TAL_LEFT
	inc = (TAL_RIGHT+TAL_LEFT)/8
	Do j=1,9
        	i = 1
        	tal(1) = f
        	tal(2) = TAL_FRONT
        	tal(3) = -TAL_BOTTOM
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = f
        	tal(2) = -(TAL_MIDDLE+TAL_BACK)
        	tal(3) = -TAL_BOTTOM
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = f
        	tal(2) = -(TAL_MIDDLE+TAL_BACK)
        	tal(3) = TAL_TOP
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = f
        	tal(2) = TAL_FRONT
        	tal(3) = TAL_TOP
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
C
C	Next slice
C
		If (j .eq. 5) then
			Call vl_draw_tal_cutplane(points,%val(1),
     +				offset,%val(zoom))
		Else
			Call vl_draw_tal_cutplane(points,%val(0),
     +				offset,%val(zoom))
		Endif
		f = f + inc
	Enddo
C
C	Now the planes parallel to XY (Z=0)
C
	f = -TAL_BOTTOM
	inc = TAL_BOTTOM/4
	Do j=1,13
        	i = 1
        	tal(1) = -TAL_LEFT 
        	tal(2) = TAL_FRONT
        	tal(3) = f
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = -TAL_LEFT 
        	tal(2) = -(TAL_MIDDLE+TAL_BACK)
        	tal(3) = f
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = TAL_RIGHT 
        	tal(2) = -(TAL_MIDDLE+TAL_BACK)
        	tal(3) = f
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = TAL_RIGHT 
        	tal(2) = TAL_FRONT
        	tal(3) = f
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
C
C	Next slice
C
		If (j .eq. 5) Then
			Call vl_draw_tal_cutplane(points,%val(1),
     +				offset,%val(zoom))
			inc = (TAL_TOP)/8
		Else
			Call vl_draw_tal_cutplane(points,%val(0),
     +				offset,%val(zoom))
		Endif
		f = f + inc
	Enddo
C
C	Now the planes parallel to XZ (Y=0)
C
	f = -(TAL_MIDDLE+TAL_BACK)
	inc = (TAL_BACK)/4
	Do j=1,12
        	i = 1
        	tal(1) = -TAL_LEFT 
        	tal(2) = f
        	tal(3) = -TAL_BOTTOM
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = -TAL_LEFT 
        	tal(2) = f
        	tal(3) = TAL_TOP
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = TAL_RIGHT 
        	tal(2) = f
        	tal(3) = TAL_TOP
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
        	tal(1) = TAL_RIGHT 
        	tal(2) = f
        	tal(3) = -TAL_BOTTOM
        	call tal_tal2point(tal,points(i),points(i+1),points(i+2))
		i = i + 3
C
C	Next slice
C
		If (j .eq. 5) Then
			Call vl_draw_tal_cutplane(points,%val(1),
     +				offset,%val(zoom))
			inc = (TAL_MIDDLE)/3
		Else if ( j .eq. 8) Then
			Call vl_draw_tal_cutplane(points,%val(1),
     +				offset,%val(zoom))
			inc = (TAL_FRONT)/4
		Else
			Call vl_draw_tal_cutplane(points,%val(0),
     +				offset,%val(zoom))
		Endif
		f = f + inc
	Enddo

	Return
	End
C
C	Routine to set the current color according to an index value
C
	Subroutine	bv_set_color(mcolor)

	Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'
	
	Integer*4	mcolor
C
C       color list
C
        Integer*4       col(14)

        data    col/'00000000'X,
     +              '000000ff'X,
     +              '0000ff00'X,
     +              '0000ffff'X,
     +              '00ff0000'X,
     +              '00ff00ff'X,
     +              '00ffff00'X,
     +              '00ffffff'X,
     +              '000080ff'X,
     +              '0000ff80'X,
     +              '0080ff00'X,
     +              '00ff8000'X,
     +              '00ff0080'X,
     +              '008000ff'X/
C
C	set color index (or color)
C
        If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then
                Call cpack(col(mcolor+1))
        Else
                Call set_color_index(col(mcolor+1))
        Endif

	Return
	End
