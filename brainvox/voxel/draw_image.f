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
C    MODULE NAME    : draw_image.f
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
C    DATE           : 20 Sep 89       
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
C    INPUTS         :          dx,dy  : data size deltas
C			       ix,iy  : output window size to fill
C			       offset : coords of LL pixel (0,dx-1)(0,dy-1)
C			       zoom   : zoom factor
C			       data   : dy lines of dx pixels
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
C
C	Integer*4 (RGB) version
C
	Subroutine draw_image_i4(dx,dy,pix,piy,offset,zoom,data,zdata)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'voxel_globals.inc'

	Integer*4	dx,dy,offset(2),zoom,data(*)
	Integer*2	zdata(*)
	Integer*4	buffer(512*512)
	Real*4		zbuffer(512*512)
	Integer*4	x,y,ix,iy,pix,piy
	Integer*4	bptr,iptr,i,j

	common  /v_buffers/buffer,zbuffer
C
C	from voxel.h
C
	Integer*4	VL_INFINITY
	Parameter	(VL_INFINITY = -32700)
C
C	set the rectzoom
C
	Call rectzo(float(zoom),float(zoom))
C
C	select proper ix and iy (if maxsize does not fill window)
C
	ix = pix
	iy = piy
	if (ix .gt. zoom*dx) ix = zoom*dx
	if (iy .gt. zoom*dy) iy = zoom*dy
C
C	x,y are the size of the array in buffer we need to blast
C
	If (zoom .le. 1) then
		x = ix
		y = iy
	Else
		x = ix/zoom
		y = iy/zoom
	Endif
C
C	copy the pixels into buffer
C
	bptr = 1
	Do J=offset(2),offset(2)+y-1
		iptr = (J*dx)
		Do I=offset(1),offset(1)+x-1
			buffer(bptr) = data(iptr+I+1)
C
C OpenGL is from [0,1]  Input from [image3dsize*4,-image3dsize*4]
C                                  32700(VL_INFINITY)]
C
			If (zdata(iptr+I+1).le.VL_INFINITY) then
C				zbuffer(bptr) = -1.0
				zbuffer(bptr) = 1.0
			Else
C				zbuffer(bptr)=((dx/2)*8)-zdata(iptr+I+1)
				zbuffer(bptr)=zdata(iptr+I+1)
				zbuffer(bptr)=zbuffer(bptr)/(8*image3dsize)
				zbuffer(bptr)=0.5 - zbuffer(bptr)
C				zbuffer(bptr)=zbuffer(bptr)/
C     +					(-8.0*image3dsize*0.5)
			Endif
			bptr = bptr + 1
		Enddo
	Enddo
C
C	do it!!
C
C	call zclear
C	call lsetde(0,dx*8)
C	call zbuffe(.false.)
	call zdraw(.true.)
C	call frontb(.true.)
C	call backbu(.true.)
	call lrectw(0,0,x-1,y-1,zbuffer)
	call zdraw(.false.)
C	call frontb(.false.)
C	call frontb(.true.)
C	call backbu(.false.)
	call zbuffe(.false.)
C
	call lrectw(0,0,x-1,y-1,buffer)
C
C	reset the rectzoom
C
	Call rectzo(1.0,1.0)
C
	Call cpack(0)
	ix = x*zoom
	iy = y*zoom
	if ((ix .ne. pix).or.(iy .ne. piy)) call blackout(ix,iy,pix,piy)
C
	Return
	End
C
C	Integer*2 (Cmap) version
C
	Subroutine draw_image_i2(dx,dy,pix,piy,offset,zoom,data1,data2,
     +			mode)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
	Include 'palettes.inc'

	Integer*4	dx,dy,offset(2),zoom,mode
	Integer*2	data1(*)
	Integer*2	data2(*)
	Integer*2	buffer(512*512*2),zbuffer(512*512*2)
	Integer*4	buffer4(512*512)
	Integer*4	x,y,ix,iy,pix,piy
	Integer*4	bptr,iptr,i,j,t

	common  /v_buffers/buffer,zbuffer
C
C	set the rectzoom
C
	Call rectzo(float(zoom),float(zoom))
C
C	select proper ix and iy (if maxsize does not fill window)
C
	ix = pix
	iy = piy
	if (ix .gt. zoom*dx) ix = zoom*dx
	if (iy .gt. zoom*dy) iy = zoom*dy
C
C	x,y are the size of the array in buffer we need to blast
C
	x = ix/zoom
	y = iy/zoom
C
C       Get visual mode
C
        If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then
C
C	copy the pixels into buffer
C
		If (mode .eq. 0) Then
			bptr = 1
			Do J=offset(2),offset(2)+y-1
				iptr = (J*dx)
				Do I=offset(1),offset(1)+x-1
				    t = pal_2d(data1(iptr+I+1)-255)
				    t = t + 
     +				    pal_2d(data1(iptr+I+1)-255+256)*256
				    t = t + 
     +				    pal_2d(data1(iptr+I+1)-255+512)*256*256
				    buffer4(bptr) = t
				    bptr = bptr + 1
				Enddo
			Enddo
		Else if (mode .eq. 1) Then
			bptr = 1
			Do J=offset(2),offset(2)+y-1
				iptr = (J*dx)
				Do I=offset(1),offset(1)+x-1
				    t = data1(iptr+I+1)-256 
				    t = t + (data2(iptr+I+1)-256)*256
				    buffer4(bptr) = blend_map1(t+1)
				    bptr = bptr + 1
				Enddo
			Enddo
		Else if (mode .eq. 2) Then
			bptr = 1
			Do J=offset(2),offset(2)+y-1
				iptr = (J*dx)
				Do I=offset(1),offset(1)+x-1
				    t = data1(iptr+I+1)-256 
				    t = t + (data2(iptr+I+1)-256)*256
				    buffer4(bptr) = blend_map2(t+1)
				    bptr = bptr + 1
				Enddo
			Enddo
		Endif
C
C	do it!!
C
		Call lrectw(0,0,x-1,y-1,buffer4)
		Call cpack(0)
C
        Else
C
C	copy the pixels into buffer
C
		bptr = 1
		Do J=offset(2),offset(2)+y-1
			iptr = (J*dx)
			Do I=offset(1),offset(1)+x-1
				buffer(bptr) = data1(iptr+I+1)
				bptr = bptr + 1
			Enddo
		Enddo
C
C	do it!!
C
		Call rectwr(0,0,x-1,y-1,buffer)
		Call color(BLACK)
C
	Endif
C
C	reset the rectzoom
C
	Call rectzo(1.0,1.0)
C
	ix = x*zoom
	iy = y*zoom
	if ((ix .ne. pix).or.(iy .ne. piy)) call blackout(ix,iy,pix,piy)
C
	Return
	End
C
C	routine to paint out unblasted area
C
	subroutine blackout(ix,iy,wx,wy)

	Implicit None

	Integer*4	ix,iy,wx,wy
C
C	get rect1 along right side
C
	if (ix .ne. wx) then
		Call sboxfi(ix,0,wx,wy)
	Endif
C
C	get rect2 along top of image
C
	if (iy .ne. wy) then
		Call sboxfi(0,iy,wx,wy)
	Endif
C
	Return
	End  
C
C
C	routine to handle the panning of images
C
	subroutine bumpoffset(offset,zoom,dx,dy,xs,ys,wx,wy)

	Implicit None

	Integer*4	offset(2),zoom,dx,dy,xs,ys,xm,ym,wx,wy
C
C	add dx,dy to offset and block to fit
C
	offset(1) = offset(1) + dx
	offset(2) = offset(2) + dy
C
C	LL pixel too far forward
C
	xm = xs-(wx/zoom)
	ym = ys-(wy/zoom)
C
	If (offset(1) .gt. xm) offset(1) = xm
	If (offset(2) .gt. ym) offset(2) = ym
C
C	LL pixel too far back
C
	If (offset(1) .lt. 0) offset(1) = 0
	If (offset(2) .lt. 0) offset(2) = 0
C
	return
	end

C
C 	routine to reproject 2D points to 3D
C
C       This routine allows the user to send the current set of 2D+Z points
C       back through the rotation matrix to generate voxel image points
C       This is VERY useful when the voxel image rotates.
C
C       preserve cutting plane status (but we must reproject with it disabled)
C
	Subroutine reproject2d3d(num2dpts,w2dpts,num3dpts,w3dpts)

	Implicit None

	Include 'voxel_globals.inc'
	Include 'windows.inc'

	Integer*4       w3dpts(4096,4)   ! x,y,?,color
	Integer*4       w2dpts(4096,4)  
	Integer*4       num3dpts,num2dpts
	Integer*4       i,tmp,retvals(4)

	tmp = wind2d(curwin).mode
	wind2d(curwin).mode = 0
C
	num3dpts = 0
	Do I=1,num2dpts
		retvals(1) = 0
		Call addpt(w2dpts(I,1),w2dpts(I,2),
     +                     num3dpts,w3dpts,w2dpts(I,3),
     +                     w2dpts(I,4),myset,image3dsize,
     +                     image3dsize,retvals)
	Enddo
	wind2d(curwin).mode = tmp

	return
	end
C
C	routine to re-render and draw the voxel image
C
	Subroutine redraw_volume(userabort,btn,num3dpts,w3dpts)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'voxel_globals.inc'
	Include '../libs/vt_inc.f'

	Integer*4	userabort,status,tempi,ix,iy,btn
	Integer*4       w3dpts(4096,4)   ! x,y,?,color
	Integer*4       num3dpts

	Integer*4       FAT,tal_over,clippingon,clipshade
	common  /fat_com/fat,clippingon,tal_over,clipshade

	tempi = winget()

	call winset(wid2)
	Call RESHAP     ! do not forget !!!
	call getsiz(ix,iy)
	Call ortho2(0.0,float(ix-1),0.0,float(iy-1))
C
	status = rendermode
	if (use_16bit .eq. 1) Then
		status = status + 16
	Endif
	if (over_render .eq. 1) Then
		status = status + 2
		over_render = 0
	Endif
	if (use_hardware .eq. 1) Then
		status = status + 512
	Endif
C
C	Normally, btn = MOUSE3
C
	Call vl_m_render(%val(1),%val(status),
     +                          %val(btn),
     +                          userabort,myset,%val(0))
C
C   Some overlay bits */
C
	Call ortho(0.0,float(ix-1),0.0,float(iy-1),
     +                  -float(image3dsize)/2.0,
     +                  float(image3dsize)/2.0)
C
	Call bumpoffset(offset3d,zoom3d,
     +                  0,0,image3dsize,image3dsize,ix,iy)
	Call draw_image_i4(image3dsize,image3dsize,ix,iy,
     +                  offset3d,zoom3d,%val(save_ptr),%val(zbufptr))
C
	Call paintpoints(num3dpts,w3dpts,-1,
     +                          zoom3d,offset3d,myset)
	if (grid1 .gt. 0.0) call draw_grid(zoom3d,gcol1,grid1)
C
C       Draw 3D cursor overlay
C
	If (clippingon .ne. 0) then
		call zbuffe(.true.)
	Else
		call zbuffe(.false.)
	Endif
	Call draw_3d_cursor(offset3d,zoom3d)
C
C       Draw Tal_overlays (if desired)
C
	If (clippingon .ne. 0) then
		call zbuffe(.true.)
	Else
		call zbuffe(.false.)
	Endif
	Call draw_tal_overlay(myset,imagesize,
     +                  tal_over,offset3d,zoom3d,gcol1)
C
	call swapbu
	call winset(tempi)

	Return
	End

