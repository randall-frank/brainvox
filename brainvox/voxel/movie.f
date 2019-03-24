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
C    MODULE NAME    : Movie!
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 03 May 96       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to display a volume as a movie/tiled view
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
	Subroutine movieola(data_id)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	
	Integer*4	data_id,status
C
C       IPC defines
C
        Integer*4       size,message(60)
C
	Integer*4	device,i,j,wid
	Integer*2	data
	Character*256	tstr
	Integer*4	the_menu
C
	Integer*4	zoom,wx,wy,inum,inc,wox,woy,flags
	Integer*4	style
C	
	Integer*4	lut(256*256),h
	Integer*4	map_addr,map_select
C
	Real*4          elapsed,tr(2),secpf
C
	Real*4          bv_etime
	External        bv_etime
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
	shm_data = data_id
        shmem_owner = 0
        Call vt_init_shmem
C
C	Init vars
C
	zoom = 1
	secpf =  0.5
	inum = 1
	inc = 1
C
C	style : 0=circular+ 1=circular- 2=ping-pong
C
	style = 0
C
C	flags : 1=tiled 2=info
C
	flags = 0
C
C	Lut?
C
	Do j=0,255
	Do i=0,255
		lut(i+(j*256)+1) = i+(i*256)+(i*65536)
	Enddo
	Enddo
C
	map_select = 0
	If (map_select .eq. 0) Then
		map_addr = lut_addr + LUT_IMG_OFFSET
	Else if (map_select .eq. 1) Then
		map_addr = lut_addr + LUT_FN1_OFFSET
	Else if (map_select .eq. 2) Then
		map_addr = lut_addr + LUT_FN2_OFFSET
	Endif
C
C       get the window
C
	wx = imagesize
	wy = imagesize
        call minsiz(wx,wy)
        wid=winope("Movieola",8)
	Call Single
        Call RGBmod
        Call gconfi
        Call qdevic(WINQUI)
C
C	The menu:
C
	the_menu = newpup()
	tstr = "Movieola %t|Circular forward %I%x1%r1|"//
     +         "Circular backward %i%x2%r1}"
        Call addtop(the_menu,tstr,index(tstr,"}")-1,0)
	tstr = "Ping-Pong %i%x3%r1%l|Verbose %i%x4|Tiled display %i%x5%l}"
        Call addtop(the_menu,tstr,index(tstr,"}")-1,0)
	tstr = "Image palette color %I%x6%r2|"//
     +          "Image texture function 1 %i%x7%r2"//
     +		"|Image texture function 2 %i%x8%r2%l}"
        Call addtop(the_menu,tstr,index(tstr,"}")-1,0)
	tstr = "Quit %x100}"
        Call addtop(the_menu,tstr,index(tstr,"}")-1,0)
C
	Call qdevic(TIMER0)
	Call qdevic(MOUSE1)
	Call qdevic(MOUSE2)
	Call qdevic(MOUSE3)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,wid)
C
	elapsed=bv_etime(tr)
C
C       redraw comes from setup_dialog
C
10      device=qtest() 			! top of event loop
	If (device .ne. 0) Then
C
C	Read pending event
C
		device=qread(data)      
	Else
C
C	Next frame in dynamic mode???
C
		If ((secpf .lt. 5.0).and.((bv_etime(tr)-elapsed) 
     +			.gt. secpf)) Then
C
C	Next image (fake a redraw)
C
			elapsed=bv_etime(tr)
			device = REDRAW
			data = 0
C
			inum = inum + inc
			If ((style .eq. 0).or.(style .eq.1)) Then
				If (inum .gt. realimages) inum = 1
				If (inum .lt. 1) inum = realimages
			Else if (style .eq. 2) Then
				If (inum .gt. realimages) Then
					inum = realimages-1
					inc = -1
				Endif
				If (inum .lt. 1) Then
					inum = 2
					inc = 1
				Endif
			Endif
C
		Else
C
C	nothing to do... (1000ticks=1sec)
C
			Call bv_sginap(5)  ! sleep 5/1000 sec
			Goto 10
		Endif
	Endif
C
C       timer stuff
C
        if (device .eq. TIMER0) then
                size = 60
                Call check_ipc(size,message)
                If (message(1) .eq. IPC_QUIT) goto 999
                goto 10
        Endif
C
C       handle redraw
C
        If (device .eq. REDRAW) then    ! redraw screen
                Call RESHAP     ! do not forget !!!
		Call getsiz(wx,wy)
                Call ortho2(0.0,float(wx),0.0,float(wy))
		If (data .eq. wid) Then
			Call cpack(0)
			Call clear
		Endif
		Call draw_frame(%val(images_addr),%val(texture_addr),
     +			imagesize,imagesize,wx,wy,zoom,%val(map_addr),
     +			map_select,inum,flags,
     +			interpimages,secpf,realimages)
		goto 10
	Endif
C
C	mouse handlers
C
        If ((device .eq.  MOUSE1) .and. (data .eq. 1)) then
C
C	menu services
C
		i = dopup(the_menu)
		If (i .eq. 100) Then     ! quit
			goto 999
		Else if (i .eq. 1) Then  ! circ+
			inc = 1
			style = 0
		Else if (i .eq. 2) Then  ! circ-
			inc = -1
			style = 1
		Else if (i .eq. 3) Then  ! ping-pong
			style = 2
		Else if (i .eq. 4) Then  ! info
			If (iand(flags,2).eq.2) then
				h = 'fffffffd'X
				flags = iand(flags,h)
			Else
				flags = ior(flags,2)
			Endif
			Call qenter(REDRAW,wid)
		Else if (i .eq. 5) Then  ! tiled
			If (iand(flags,1).eq.1) then
				h = 'fffffffe'X
				flags = iand(flags,h)
			Else
				flags = ior(flags,1)
			Endif
			Call qenter(REDRAW,wid)
		Else if ((i .ge. 6).and. (i .le. 8)) Then  ! new mapping
			map_select = i-6
			If (map_select .eq. 0) Then
				map_addr = lut_addr + LUT_IMG_OFFSET
			Else if (map_select .eq. 1) Then
				map_addr = lut_addr + LUT_FN1_OFFSET
			Else if (map_select .eq. 2) Then
				map_addr = lut_addr + LUT_FN2_OFFSET
			Endif
			Call qenter(REDRAW,wid)
		Endif
		goto 10
	Endif
        If ((device .eq.  MOUSE2) .and. (data .eq. 1)) then
C
C	Speed change
C
		i=getval(MOUSEX)
		Call getori(wox,woy)
		i = i - wox
		If (i .lt. 0) i = 0
		If (i .gt. wx) i = wx
		secpf = 2.8*(float(i)/float(wx))
		secpf = 10.0**(secpf-2.0)
		If (iand(flags,2).eq.2) Call qenter(REDRAW,0)
		goto 10
	Endif
        If ((device .eq.  MOUSE3) .and. (data .eq. 1)) then
C
C	interactive paging
C
		Do while(getbut(MOUSE3))
			i=getval(MOUSEX)
			Call getori(wox,woy)
			i = i - wox
			If (i .lt. 0) i = 0
			If (i .gt. wx) i = wx
			i = jifix((float(i)/float(wx))*float(realimages))+1
			If (i .lt. 1) i = 1
			If (i .gt. realimages) i = realimages
			If (i .ne. inum) Then
				inum = i
C
C	Redraw
C
				Call draw_frame(%val(images_addr),
     +					%val(texture_addr),imagesize,
     +					imagesize,wx,wy,zoom,%val(map_addr),
     +					map_select,inum,
     +					flags,interpimages,secpf,realimages)
			Endif
		Enddo
		goto 10
	Endif
C
C	get next event
C
	goto 10
C
C	Exit 
C
999	Call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)

	Return
	End
C
C	The drawing routine
C
	Subroutine draw_frame(idata,tdata,dx,dy,wx,wy,zoom,lut,map_select,
     +		inum,flags,interp,secpf,nimgs)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'

	Real*4		secpf
	Integer*4	dx,dy,wx,wy,zoom,lut(*),inum,flags,interp
	Integer*4	map_select,h
	Integer*4	i,base,j,k,x,y,nx,ny,ix,iy,jnum,bx,by,nimgs
C
	Character*200	tstr
C
	Byte		idata(*),tdata(*)
C
C	image buffer
C
	Integer*4	buf(512*512)
C
C	make it static
C
	Common	/dummy/buf
C
	jnum=inum
C
C	Compute the number of visible images 
C
	If (iand(flags,1).eq.1) Then
		nx = wx/(dx*zoom)
		ny = wy/(dy*zoom)
	Else
		nx = 1
		ny = 1
	Endif
C
C	lower left corner
C
	bx = (wx-(zoom*dx*nx))/2
	by = (wy-(zoom*dy*ny))/2
C
C	Display the tiled images
C
	Call rectzo(float(zoom),float(zoom))
	Do iy=1,ny
	Do ix=1,nx
C
C	copy through the LUT
C
	base = (jnum-1)*((interp+1)*dx*dy)
	If (map_select .eq. 0) Then
		Do i=1,dx*dy
			j = idata(base+i)
			if (j .lt. 0) j = 256 + j
			buf(i) = lut(j+1)
		Enddo
	Else
		Do i=1,dx*dy
			j = idata(base+i)
			k = tdata(base+i)
			if (j .lt. 0) j = 256 + j
			if (k .lt. 0) k = 256 + k
			buf(i) = lut(j+(k*256)+1)
		Enddo
	Endif
C
C	bit-blt
C
	x = bx+(zoom*dx)*(ix-1)
	y = by+(zoom*dy)*(ny-iy)
	Call lrectw(x,y,x+dx-1,y+dy-1,buf)
C
	jnum = jnum + 1
	If (jnum .gt. nimgs) jnum = 1
C
C	next image
C
	Enddo
	Enddo
C
	Call rectzo(1.0,1.0)
C
	If (iand(flags,2).eq.2) Then
		Call cpack(0)
		Call sboxfi(0,0,170,18+15)
		h = '00ffffff'X
		Call cpack(h)
		write(tstr,10) inum
10	Format("Slice: ",I3,"}")
		Call cmov2i(3,18)
		Call charst(tstr,index(tstr,"}")-1)
		If (secpf .gt. 5.0) Then
			tstr = "Stopped}"
		Else
			write(tstr,20) secpf
20	Format("Sec/frame: ",F6.4,"}")
		Endif
		Call cmov2i(3,3)
		Call charst(tstr,index(tstr,"}")-1)
	Endif
C
	Call gflush
C
	Return
	End
