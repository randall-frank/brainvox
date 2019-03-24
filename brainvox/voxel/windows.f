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
C    MODULE NAME    : Windows 
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
C    DATE           : 22 mar 94       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to work with multiple 2D windows
C	Support for an active window list and tools for swapping in/out
C	various window variables
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
	Subroutine init_window(wid)

	Implicit None

	Include 'windows.inc'

	Integer*4	wid
	Integer*4	i
C
C	Clear out the window list
C
	Do i=1,MAX_WINDOWS
		wind2d(i).wid = -1
		wind2d(i).im2d_ptr = 0
		wind2d(i).tex2d_ptr = 0
	Enddo
C
C	First call will fill in slot (1)
C
	Call add_window(wid)
C
C	init the curwin ptr
C
	Call sel_window(1,2,0,0)
C
C	3D cursor initialization
C	display = off
C	tracking = off
C	location = 0,0,0
C	color = red
C
	cursor3d_disp = 0
C
	cursor3d_track = 0
C
	cursor3d_pos(1) = 0
	cursor3d_pos(2) = 0
	cursor3d_pos(3) = 0
C
	cursor3d_pos(4) = 1
C
	cursor3d_mode = 1+4  ! lines + wire planes
C
	Return
	End
C
C	Find a slot (pass -1 as wid to find next open slot)
C
	Subroutine	find_window(wid,loc)
	
	Implicit None

	Include 'windows.inc'

	Integer*4	i,wid,loc

	Do i=1,MAX_WINDOWS
		If (wind2d(i).wid .eq. wid) Then
			loc = i
			Return
		Endif
	Enddo
	loc = -1
	Return
	End
C
C	Initialize a window structure
C
	Subroutine	start_window(i,wid)

	Implicit None

	Include 'windows.inc'
	Include '../libs/vt_inc.f'

	Integer*4	i,wid,j

	wind2d(i).wid = wid
	wind2d(i).offset2d(1) = 0
	wind2d(i).offset2d(2) = 0
	wind2d(i).grid = 0.0
	wind2d(i).gcol = 0
	wind2d(i).zoom2d = 1
	wind2d(i).mode = 0   ! cuttingon-0-slice 1-arb 2-MP_FIT
	wind2d(i).inum2d = 1
	wind2d(i).n_slice = 0
	wind2d(i).dirty = .true.
	wind2d(i).texture_mode = 0
	wind2d(i).cursor_locked = 0
	call fmalloc(wind2d(i).im2d_ptr,imagesize*imagesize*2)
	call fmalloc(wind2d(i).tex2d_ptr,imagesize*imagesize*2)
	Call memclear(%val(wind2d(i).im2d_ptr),imagesize*imagesize*2)
	Call memclear(%val(wind2d(i).tex2d_ptr),imagesize*imagesize*2)
	Do j=1,3
		wind2d(i).rots(j) = 0
	Enddo
	wind2d(i).rots(4) = 256
C
	Return
	End

	Subroutine add_window(wid,myset,rots)

	Implicit None

	Include 'fgl.h'
	Include '../libs/vt_inc.f'
	Include 'windows.inc'
	Include 'rotsdef.inc'

	Record  /rotsrec/       rots

	Integer*4	wid,i,myset(*)
	Character*20	str
C
C	Find the window
C
	Call find_window(-1,i)
	If (i .lt. 0) return
C
C	Open a window on screen
C	
	write(str,1) i
1	Format("2DImage",I2.2)
	If (i .eq. 1) Then
		Call minsiz(imagesize,imagesize)
		Call prefpo2(445+20,(getgde(GDYPMA)-imagesize)+imagesize-1)
C		Call prefpo(445+20,445+20+imagesize-1,
C     +			(getgde(GDYPMA)-imagesize),
C     +			(getgde(GDYPMA)-imagesize)+imagesize-1)
	Else
		Call minsiz(imagesize,imagesize)
	Endif
	wid = winope(str,9)
	if (i .eq. 1) then
		Call minsiz(imagesize,imagesize)
		Call wincon
	Endif
	Call doubleb
	Call rgbmod
	If (getgde(GDBIST) .ne. 0) Then
		Call stensi(0)
	Endif
	Call gconfi
C
C	Drop in values
C
	Call start_window(i,wid)
C
C	New current window
C
	Call sel_window(i,1,myset,rots)
C
	Call rdr_window(i) 
	Call dirty_window(i,1)
C
	Return
	End
C
C	Set current window (type=1=full user visible swap,0=temp swap)
C
	Subroutine sel_window(i,type,myset,rots)

	Implicit None

	Include 'windows.inc'
	Include 'fgl.h'
	Include '../libs/vt_inc.f'
	Include 'rotsdef.inc'
	
	Integer*4	myset(*)
	Integer*4	i,win,type,status
	Real*4		frots(10)
	Character*200	str

	Record  /rotsrec/       rots

1	Format("2DImage",I2.2,"}")
2	Format("<2DImage",I2.2,">}")

	If ((i .gt. MAX_WINDOWS).or.(i .lt. 1)) return

	If (type .eq. 1) Then	
		win = winget()
		If ((curwin .ge. 1).and.(curwin .le. MAX_WINDOWS)) Then
			Call winset(wind2d(curwin).wid)
			write(str,1) curwin
			Call wintit(str,index(str,"}")-1)
		Endif

		curwin = i
		Call winset(wind2d(curwin).wid)
		write(str,2) curwin
		Call wintit(str,index(str,"}")-1)

		Call winset(win)

		Call update_window_menu(-2)
		Call send_image_histo
	Endif
C
C	temporary stuff
C
	curwin = i
	if (type .eq. 2) return
C
C       Setup vectors for pointset clipping
C
        If (wind2d(curwin).mode .eq. 1) Then
                Do status = 1,3
                        frots(status) = wind2d(curwin).rots(status)
                Enddo
                frots(4) = wind2d(curwin).rots(4)-256
                Call vl_sample_vectors(%val(imagesize),
     +                  %val(imagesize),frots(1),frots(4),myset)
        Else if (wind2d(curwin).mode .eq. 2) Then
                frots(4) = rots.sthickness/interpixel
                Do status=1,3
                        frots(status)=rots.upcenter(status) +
     +                          float(-wind2d(curwin).n_slice)*
     +                          frots(4)*rots.upnorm(status)
                Enddo
                Call vl_mpsample_vectors(%val(imagesize),
     +                  %val(imagesize),rots.upxaxis(1),
     +                  rots.upyaxis(1),rots.upnorm(1),
     +                  frots(1),myset)
        Endif

	Return
	End
C
C	Remove a window (wid)
C
	Subroutine rem_window(wid,myset,rots)

	Implicit None

	Include 'windows.inc'
	Include 'fgl.h'
	Include 'rotsdef.inc'

	Record  /rotsrec/       rots

	Integer*4	wid,myset(*)
	Integer*4	i
C
	If (wid .eq. -1) return
C
C	Find the window
C
	Call find_window(wid,i)
	If (i .le. 1 ) return  ! cannot remove window 1 or one not found
C
	If (curwin .eq. i) Then
		Call sel_window(1,1,myset,rots)
	Endif
C
C	Destroy the window
C
	Call winclo(wid)
	wind2d(i).wid = -1
	Call ffree(wind2d(i).im2d_ptr)
	wind2d(i).im2d_ptr = 0
	Call ffree(wind2d(i).tex2d_ptr)
	wind2d(i).tex2d_ptr = 0
C
	Return
	End
C
C	Routine to ditry one (or more) windows
C	Which = window number to dirty or -1 for all
C	dirty=1 for all windows, 2=only MP_FIT windows
C
	Subroutine dirty_window(which,dirty)

	Implicit None

	Include 'windows.inc'

	Integer*4	which,i
	Integer*4	dirty

	If (which .eq. -1) Then
		Do i=1,MAX_WINDOWS
			If (wind2d(i).wid .ne. -1) Then
				If (dirty .eq. 1) Then
					wind2d(i).dirty = .true.
				Else if ((dirty .eq. 2) .and.
     +			(wind2d(i).mode .eq. 2)) Then
					wind2d(i).dirty = .true.
				Endif
			Endif
		Enddo
	Else
		i = which
		If (dirty .eq. 1) Then
			wind2d(i).dirty = .true.
		Else if ((dirty .eq. 2).and.(wind2d(i).mode .eq. 2)) Then
			wind2d(i).dirty = .true.
		Endif
	Endif

	Return
	End
C
C	Routine to queue redraws for all or current 2D window
C
	Subroutine rdr_window(which)

	Implicit None

	Include 'windows.inc'
	Include 'fgl.h'
	Include 'fdevice.h'
	
	Integer*4	which,i

	If (which .eq. -1) Then
		Do i=1,MAX_WINDOWS
			If (wind2d(i).wid .ne. -1) Then
				Call qenter(REDRAW,wind2d(i).wid)
			Endif
		Enddo
	Else
		Call qenter(REDRAW,wind2d(curwin).wid)
	Endif

	Return
	End
C
C	Routine to update the window popup menu
C
	Subroutine	update_window_menu(wid)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'windows.inc'

	Integer*4	i,wid,ll,inc
	Character*100	tstr

	Integer*4 bv_os
	External bv_os
C
	inc = 0
	if (bv_os() .eq. 0) inc = 1
C
C	Now, the FIRST window cannot be removed, and the
C	current window must be selected.
C	
	If (wid .gt. -1) Then
C
		Call find_window(wid,i)
		If (i .gt. 1) Then
			Call setpup(im_menu2,3,PUPNON)
		Else
			Call setpup(im_menu2,3,PUPGRE)
		Endif
C
		If (i .ne. curwin) Then
			Call setpup(im_menu2,1,PUPNON)
		Else
			Call setpup(im_menu2,1,PUPGRE)
		Endif
C
		Call find_window(-1,i)
		If (i .gt. 0) Then
			Call setpup(im_menu2,2,PUPNON)
		Else
			Call setpup(im_menu2,2,PUPGRE)
		Endif
C
C	Set the texture mode items
C
		Call find_window(wid,i)
100	Format("Image palette color %",A1,"%r1%x8000}")
101	Format("Image texture function 1 %",A1,"%r1%x8001}")
102	Format("Image texture function 2 %l%",A1,"%r1%x8002}")
103	Format("Track 3D cursor %",A1,"%x8003}")
		If (wind2d(i).texture_mode .eq. 0) Then
			write(tstr,100) "I"
		Else
			write(tstr,100) "i"
		Endif
		ll = index(tstr,"}")-1
		Call change(im_menu2,12+inc,tstr,ll,0)
		If (wind2d(i).texture_mode .eq. 1) Then
			write(tstr,101) "I"
		Else
			write(tstr,101) "i"
		Endif
		ll = index(tstr,"}")-1
		Call change(im_menu2,13+inc,tstr,ll,0)
		If (wind2d(i).texture_mode .eq. 2) Then
			write(tstr,102) "I"
		Else
			write(tstr,102) "i"
		Endif
		ll = index(tstr,"}")-1
		Call change(im_menu2,14+inc,tstr,ll,0)
		If (wind2d(i).cursor_locked .ne. 0) Then
			write(tstr,103) "I"
		Else
			write(tstr,103) "i"
		Endif
		ll = index(tstr,"}")-1
		Call change(im_menu2,15+inc,tstr,ll,0)
C
	Endif
C
	Do i=1,MAX_WINDOWS
		If (wind2d(i).wid .ne. -1) Then
			If (i .eq. curwin) Then
				write(tstr,2) i,9100+i
			Else
				write(tstr,1) i,9100+i
			Endif
			ll = index(tstr,"}")-1
			Call change(windows_menu,i,tstr,ll,0)
			Call setpup(windows_menu,i,PUPNON)
		Else
			write(tstr,1) i,9100+i
			ll = index(tstr,"}")-1
			Call change(windows_menu,i,tstr,ll,0)
			Call setpup(windows_menu,i,PUPGRE)
		Endif

1	Format("2DImage",I2.2," %i%x",I4.4,"}")
2	Format("2DImage",I2.2," %I%x",I4.4,"}")

	Enddo
C
	Return
	End
C
C	compute the histo field
C
	Subroutine calc_image_histo(which)

	Implicit None

	Include 'windows.inc'
	Include '../libs/vt_inc.f'

	Integer*4	which,i

	Do i=1,257
		wind2d(which).histo(i) = 0
		wind2d(which).thisto(i) = 0
	Enddo

        Call count_pixels(imagesize*imagesize,wind2d(which).histo,
     +          %val(wind2d(which).im2d_ptr))

	If (wind2d(which).texture_mode .ne. 0) Then
        	Call count_pixels(imagesize*imagesize,wind2d(which).thisto,
     +          	%val(wind2d(which).tex2d_ptr))
	Endif

        Return
        End

        Subroutine count_pixels(num,histo,image)

        Implicit None

        Integer*4       i,num,histo(257)
        Integer*2       image(*)

        Do i=1,num
                histo(257) = histo(257) + 1
                histo(image(i)-255) = histo(image(i)-255) + 1
        Enddo

        Return
        End
C
C	send the current image histogram to the palette editor
C
	Subroutine send_image_histo

	Implicit None

	Include 'windows.inc'
	Include '../libs/vt_inc.f'

	Integer*4	message(IPC_SHM_SIZE),size,i

        message(1) = IPC_I_HISTO
	size = 258
	Do i=1,256
		message(i+1) = wind2d(curwin).histo(i)
	Enddo
	message(258) = IPC_H_MRICUT
        Call send_ipc(size,message)

        message(1) = IPC_I_HISTO
	size = 258
	Do i=1,256
		message(i+1) = wind2d(curwin).thisto(i)
	Enddo
	message(258) = IPC_H_TEXTURECUT
        Call send_ipc(size,message)

	Return
	End
