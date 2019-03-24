
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
C    MODULE NAME    : 3Dcursor_overlays
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
C    DATE           : 13 Jun 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to implement 3D cursor overlays
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

C#define XMIN	extents(1)
C#define XMAX	extents(2)
C#define YMIN	extents(3)
C#define YMAX	extents(4)
C#define ZMIN	extents(5)
C#define ZMAX	extents(6)

C
C       Subroutine to setup the ortho params and draw the 3Dcursor
C       graphics into the voxel image window (3d)
C
        Subroutine draw_3d_cursor(offset,zoom)

        Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'windows.inc'
	Include 'voxel_colors.inc'
	Include 'voxel_globals.inc'
	Include '../libs/vt_inc.f'

        Integer*4               offset(2),zoom
C
        Integer*4               a,b
        Real*4                  dx,dy
        Real*4                  oth(6)
	Integer*4		i,plane
	Integer*4		p(6*3)
	Integer*4		extents(6)
	Integer*4		fudge
	Parameter		(fudge = 5)
C
	If (cursor3d_disp .eq. 0) Return
C
C	set color index (or color)
C
C	Call bv_set_color(cursor3d_pos(4))
C
C       Get windowsize/2
C
        Call getsiz(a,b)
        dx = float(a)/float(zoom)
        dy = float(b)/float(zoom)
        a = image3dsize/2
        b = image3dsize/2
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
        oth(5) = -(image3dsize/2)
        oth(6) = (image3dsize/2)
C
        Call ortho(oth(1),oth(2),oth(3),oth(4),oth(5),oth(6))
C
C	Get the volume extents
C
	Call vl_getextents(extents,myset)
C
C	Textured cutting planes (always drawn last as this screws with
C		the zbuffer mode)
C
	If (iand(cursor3d_mode,2) .eq. 2) Then
C
C	new zbuffer function
C
		If (getgde(GDBIST) .ne. 0) Then
C
C	Use stencil plane
C
		    Do plane=1,3
C	Clear
			Call sclear(0)
C	Disable z and pixel drawing
			Call zwrite('C0000000'X)
			Call wmpack(0)
C	Enable stencil to add where the planes are drawn
			Call stenci(.true.,0,SFALWA,3,STKEEP,STKEEP,STINCR)
C	Draw one
			Call zfunct(ZFGREA)
			Call Draw_Cursor_planes(1,extents,myset,plane)
C	Shift zbuffer (artificially)
			Call lsetde(10,(image3dsize*8)+10)
C	Draw two
			Call zfunct(ZFLESS)
			Call Draw_Cursor_planes(1,extents,myset,plane)
C	Unshift
			Call lsetde(-10,(image3dsize*8)-10)
C	Use the stencil plane (only draw if != 1, check all planes (3))
			Call stenci(.true.,1,SFNOTE,3,STKEEP,STKEEP,STKEEP)
C	Enable pixel and zbuffers
			Call wmpack('ffffffff'X)
			Call zwrite('ffffffff'X)
C	Draw for real 
C	Z clipping not needed (stenciling should do the clipping job)
			Call zbuffe(.false.)
			Call Draw_Cursor_planes(1,extents,myset,plane)
			Call zbuffe(.true.)
C	Disable stencil planes
			Call stenci(.false.,0,0,0,0,0,0)
C	Re-enable the z function
			Call zfunct(ZFLEQU)
			Call lsetde(0,(image3dsize*8))

		    Enddo
C
		Else
C
C	fake it
C
			Call zfunct(ZFEQUA)
			Call Draw_Cursor_planes(1,extents,myset,0)
			Call zfunct(ZFLESS)
		Endif
C
	Endif
C
C	drawing modes bitmask (lines=1, planes=2)
C
	If (iand(cursor3d_mode,1) .eq. 1) Then
C
C	Compute the points 
C
		i = 1
		p(i+0) = extents(1) - fudge
		p(i+1) = cursor3d_pos(2)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = extents(2) + fudge
		p(i+1) = cursor3d_pos(2)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(3) - fudge
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(4) + fudge
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(5) - fudge
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(6) + fudge
		i = i + 3
C
C	Draw lines
C
		Call bv_set_color(BV_COLOR_RED)
		Call vl_draw_pset_line(p(1),1,myset) ! x,y,z(1),x,y,z(2), line2
		Call bv_set_color(BV_COLOR_GREEN)
		Call vl_draw_pset_line(p(7),1,myset) ! x,y,z(1),x,y,z(2), line2
		Call bv_set_color(BV_COLOR_BLUE)
		Call vl_draw_pset_line(p(13),1,myset) ! x,y,z(1),x,y,z(2), line2
C
	Endif
C
C	Outer boundary lines
C
	If (iand(cursor3d_mode,4) .eq. 4) Then
		Call Draw_Cursor_planes(0,extents,myset,0)
	Endif
C
        Return
        End
C
C	Routine to draw the 3D cursor on an arbcut plane
C
        Subroutine draw_3d_cursor_arb(offset,zoom,mode)

        Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'windows.inc'
	Include 'voxel_colors.inc'
	Include '../libs/vt_inc.f'

        Integer*4               offset(2),zoom,mode
C
	Integer*4		i
	Integer*4		p(3*4)
	Integer*4		fudge,dist,orig
	Parameter		(fudge = 5)
C
	If (cursor3d_disp .eq. 0) Return
C
C	set color index (or color)
C
C	Call bv_set_color(cursor3d_pos(4))
C
C	MP_FIT mode (not for now)
C
	If (mode .eq. 2) Then
		Return
	Endif
C
C	slice plane mode
C
	If (mode .ne. 1) Then
C
C	Get the lines
C
		i = 1
		p(i+0) = 0
		p(i+1) = (cursor3d_pos(2)-offset(2))*zoom
		i = i + 3
		p(i+0) = imagesize*zoom
		p(i+1) = (cursor3d_pos(2)-offset(2))*zoom
		i = i + 3
		p(i+0) = (cursor3d_pos(1)-offset(1))*zoom
		p(i+1) = 0
		i = i + 3
		p(i+0) = (cursor3d_pos(1)-offset(1))*zoom
		p(i+1) = imagesize*zoom
		i = i + 3
C
C	Draw the lines
C
		Call bv_set_color(BV_COLOR_GREEN)
		Call bgnlin
		Call v2i(p(1))
		Call v2i(p(4))
		Call endlin
		Call bv_set_color(BV_COLOR_RED)
		Call bgnlin
		Call v2i(p(7))
		Call v2i(p(10))
		Call endlin

		Return
	Endif
C
C	Arbitrary mode
C
C	Collect the points making up the plane (3D rect)
C
C	X plane
C
	dist = imagesize*2
	orig = -imagesize
	i = 1
	p(i+0) = cursor3d_pos(1)
	p(i+1) = orig
	p(i+2) = orig
	i = i + 3
	p(i+0) = cursor3d_pos(1)
	p(i+1) = dist
	p(i+2) = orig
	i = i + 3
	p(i+0) = cursor3d_pos(1)
	p(i+1) = dist
	p(i+2) = dist
	i = i + 3
	p(i+0) = cursor3d_pos(1)
	p(i+1) = orig
	p(i+2) = dist
	i = i + 3
	Call bv_set_color(BV_COLOR_RED)
	Call vl_draw_tal_cutplane(p,%val(1),offset,%val(zoom))
C
C	Y plane
C
	i = 1
	p(i+0) = orig
	p(i+1) = cursor3d_pos(2)
	p(i+2) = orig
	i = i + 3
	p(i+0) = dist
	p(i+1) = cursor3d_pos(2)
	p(i+2) = orig
	i = i + 3
	p(i+0) = dist
	p(i+1) = cursor3d_pos(2)
	p(i+2) = dist
	i = i + 3
	p(i+0) = orig
	p(i+1) = cursor3d_pos(2)
	p(i+2) = dist
	i = i + 3
	Call bv_set_color(BV_COLOR_GREEN)
	Call vl_draw_tal_cutplane(p,%val(1),offset,%val(zoom))
C
C	Z plane
C
	i = 1
	p(i+0) = orig
	p(i+1) = orig
	p(i+2) = cursor3d_pos(3)
	i = i + 3
	p(i+0) = dist
	p(i+1) = orig
	p(i+2) = cursor3d_pos(3)
	i = i + 3
	p(i+0) = dist
	p(i+1) = dist
	p(i+2) = cursor3d_pos(3)
	i = i + 3
	p(i+0) = orig
	p(i+1) = dist
	p(i+2) = cursor3d_pos(3)
	i = i + 3
	Call bv_set_color(BV_COLOR_BLUE)
	Call vl_draw_tal_cutplane(p,%val(1),offset,%val(zoom))
C
	Return
	End
C
C	Handle menu cursor options
C
	Subroutine do_cursor3d(menu,col)

	Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'windows.inc'

	Integer*4	menu,col

	If (menu .eq. 9200) Then
		cursor3d_disp = 1 - cursor3d_disp
		Call qenter(REDRAW,0)
	Else if (menu .eq. 9201) Then
		cursor3d_track = 1 - cursor3d_track
	Else if (menu .eq. 9202) Then
		cursor3d_pos(4) = col
		Call qenter(REDRAW,0)
	Else if (menu .eq. 9203) Then		
		If (iand(cursor3d_mode,2) .eq. 2) then
			cursor3d_mode = cursor3d_mode - 2
		Else 
			cursor3d_mode = cursor3d_mode + 2
		Endif
		Call qenter(REDRAW,0)
	Endif

	Return
	End
C
C	Routine to handle the "locked" windows
C
	Subroutine pan_locked_windows(myset)

	Implicit None

	Include 'windows.inc'
	Include '../libs/vt_inc.f'

	Integer*4	myset(*)
	Integer*4	i,inum
	Real*4		v,p(3)
	Real*4          squeeze
C
	Integer*4	ZtoSlice
	External	ZtoSlice
C
C	build point
C
	Call vl_getsqu(squeeze,myset)
	inum = realimages + (realimages-1)*interpimages
	p(1) = cursor3d_pos(1)-(0.5*imagesize)
	p(2) = cursor3d_pos(2)-(0.5*imagesize)
	p(3) = (cursor3d_pos(3)-(0.5*inum))*squeeze
C
C	loop through all the windows
C	
	Do i=1,MAX_WINDOWS
C
C	must be active
C
		If (wind2d(i).wid .ne. -1) Then
C
C	must be cursor locked
C
		If (wind2d(i).cursor_locked .ne. 0) Then
C
C	handle slice or arb mode
C
		If (wind2d(i).mode .eq. 0) Then  ! slice mode
			wind2d(i).inum2d = ZtoSlice(cursor3d_pos(3))
			wind2d(i).dirty = .true.
		Else if (wind2d(i).mode .eq. 1) Then  ! arbitrary mode
			Call match_plane(wind2d(i).rots(1),p,v)
			wind2d(i).rots(4)=(v+256.5)
			wind2d(i).dirty = .true.
		Endif

		Endif
		Endif
	Enddo
C
C	No redraw needed (assuming caller did this!!)
C
	Return
	End
C
C	draw 3D solids or wires
C
	Subroutine	Draw_Cursor_planes(solid,extents,myset,num)

	Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'fget.h'

        Include 'rotsdef.inc'
	Include 'windows.inc'
	Include 'voxel_colors.inc'
	Include '../libs/vt_inc.f'
	
	Integer*4	solid,i
	Integer*4	p(6*3)
	Integer*4	extents(6)
	Integer*4	myset,num

C
	If ((num .eq. 0).or.(num .eq. 1)) Then
C
C	X plane
C
		i = 1
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(3)
		p(i+2) = extents(5)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(4)
		p(i+2) = extents(5)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(4)
		p(i+2) = extents(6)
		i = i + 3
		p(i+0) = cursor3d_pos(1)
		p(i+1) = extents(3)
		p(i+2) = extents(6)
		i = i + 3
		Call bv_set_color(BV_COLOR_RED)
C
C	Draw lines or polys
C
		If (solid .eq. 1) Then
			Call vl_draw_pset_poly(p,4,myset)
		Else
			p(i+0) = p(1)
			p(i+1) = p(2)
			p(i+2) = p(3)
			i = i + 3
			Call vl_draw_pset_line(p(1),1,myset) 
			Call vl_draw_pset_line(p(4),1,myset) 
			Call vl_draw_pset_line(p(7),1,myset) 
			Call vl_draw_pset_line(p(10),1,myset) 
		Endif
	Endif
C
	If ((num .eq. 0).or.(num .eq. 2)) Then
C
C	Y plane
C
		i = 1
		p(i+0) = extents(1)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(5)
		i = i + 3
		p(i+0) = extents(2)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(5)
		i = i + 3
		p(i+0) = extents(2)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(6)
		i = i + 3
		p(i+0) = extents(1)
		p(i+1) = cursor3d_pos(2)
		p(i+2) = extents(6)
		i = i + 3
		Call bv_set_color(BV_COLOR_GREEN)
C
C	Draw lines or polys
C
		If (solid .eq. 1) Then
			Call vl_draw_pset_poly(p,4,myset)
		Else
			p(i+0) = p(1)
			p(i+1) = p(2)
			p(i+2) = p(3)
			i = i + 3
			Call vl_draw_pset_line(p(1),1,myset) 
			Call vl_draw_pset_line(p(4),1,myset) 
			Call vl_draw_pset_line(p(7),1,myset) 
			Call vl_draw_pset_line(p(10),1,myset) 
		Endif
	Endif
C
	If ((num .eq. 0).or.(num .eq. 3)) Then
C
C	Z plane
C
		i = 1
		p(i+0) = extents(1)
		p(i+1) = extents(3)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = extents(2)
		p(i+1) = extents(3)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = extents(2)
		p(i+1) = extents(4)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		p(i+0) = extents(1)
		p(i+1) = extents(4)
		p(i+2) = cursor3d_pos(3)
		i = i + 3
		Call bv_set_color(BV_COLOR_BLUE)
C
C	Draw lines or polys
C
		If (solid .eq. 1) Then
			Call vl_draw_pset_poly(p,4,myset)
		Else
			p(i+0) = p(1)
			p(i+1) = p(2)
			p(i+2) = p(3)
			i = i + 3
			Call vl_draw_pset_line(p(1),1,myset) 
			Call vl_draw_pset_line(p(4),1,myset) 
			Call vl_draw_pset_line(p(7),1,myset) 
			Call vl_draw_pset_line(p(10),1,myset) 
		Endif
	Endif
C
	Return
	End
