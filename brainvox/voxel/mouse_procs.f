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
C    MODULE NAME    : Mouse_procs
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
C    DATE           : 19 Jun 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to handle the mousedown events
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
	Subroutine handle_mousedown(device,data,rots,dialog,
     +		MP_disp,cindex,w2dpts,w3dpts,num2dpts,num3dpts,frots,
     +		handled,dirty)
C
	Implicit None
	
	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'voxeluif.inc'
	Include 'keyboard_inc.f'

	Include 'measure.inc'
	Include 'rotsdef.inc'
	Include 'voxel_globals.inc'
	Include 'windows.inc'
	Include 'prefs.f'
C
C	Parameters:
C
	Record	/rotsrec/	rots
	Record	/ditem/	dialog(100)
	Integer*4	MP_disp
	Integer*4	handled
	Integer*4	device
	Integer*2	data
	Integer*4	cindex
	Real*4		frots(12),pps
	Integer*4	ms_per_point
	Integer*4	w2dpts(4096,4)   ! x,y,slice,color
	Integer*4	w3dpts(4096,4)   ! x,y,?,color
	Integer*4	num2dpts,num3dpts
	Logical		dirty
C	
C 	Local:
C
	Integer*4	tx,ty,winsave
	Integer*4	i,status,tmpi
	Integer*4	wleft,wbot,xo,yo,x,y
	Integer*4	ix,iy,j,retvals(4),depth
	Integer*4	modifiers
C
C	Functions
C
	Integer*4	slicetoz
	External	slicetoz
C
C	Return values
C
	handled = 0  ! passthrough
C
C       A mouse button?
C
        if ((device .ne. MOUSE1).and.(device .ne. MOUSE2).and.
     +     (device .ne. MOUSE3)) Return
C
C	check for scrolling on either image
C
	if (data .eq. 1) then
C
C	which window????
C
		i = -1
		if (winput .eq. wid2) i = 3
		Call find_window(winput,status)
		if (status .ge. 1) i = 2
C
C	Rotate the cube view
C
		If (winput .eq. wid) then
C
C	dynamic cube view 
C	get window pos
			call getori(wleft,wbot)
C	get window x,y
			xo = getval(CURSRX)-wleft
			yo = getval(CURSRY)-wbot
			if ((xo .lt. 4) .or. (yo .lt. 4) .or.
     +				(xo .gt. 106) .or. (yo .gt. 106)) Return
C
			Do while (getbut(device))
C	get new x,y
				x = getval(CURSRX)-wleft
				y = getval(CURSRY)-wbot
				If (getbut(LEFTAL).or.getbut(RGHTAL).or.
     +				   getbut(CTRLKE) .or. getbut(RIGHTC).or. 
     +					keyboard_query(KEYBOARD_ALT)) Then
					ix = 0
					iy = 0
					j = x-xo
				Else
					ix = x-xo
					iy = y-yo
					j = 0
				Endif
				If ((ix .ne. 0).or.(iy.ne.0).or.
     +					(j.ne.0)) then
					call cube_rot(ix,iy,j,myset,frots)
					call frontb(.true.)
					call draw_cube(myset,rots,MP_disp)
					Call ortho2(0.0,485.0,0.0,385.0)
					call gfflush
					call frontb(.false.)
					If (auto_render .eq. 1) Then
					    Call reproject2d3d(num2dpts,
     +							w2dpts,num3dpts,w3dpts)
					    Call redraw_volume(tmpi,0,
     +							num3dpts,w3dpts)
					Endif
				Endif
				xo = x
				yo = y
			Enddo
C
C	Change view (w/cube)
C
			call cube_rot(0,0,0,myset,frots)
			Do status=1,3
				Do while (frots(status) .lt. 0.0)
					frots(status)=frots(status)+360.0
				Enddo
				Do while (frots(status) .ge. 360.0)
					frots(status)=frots(status)-360.0
				Enddo
				rots.rots(status) = frots(status)
			Enddo
			Call setup_dialog(rots,dialog,wid,myset,wid2)
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
			goto 7001
		Endif
C
C	if improper window loop back...
c
		if (i .eq. -1) goto 10
C
C	Down in "pan" window...
C
                modifiers = 0
	        If (getbut(LEFTSH) .or. getbut(RIGHTS).or.
     +		    keyboard_query(KEYBOARD_SHIFT)) modifiers = 1
C
		if ((device .eq. MOUSE2).or.((modifiers .eq. 1).and.
     +              (device .eq. MOUSE1))) then
C
C	tempory window focus swap
C
			winsave = curwin
			Call sel_window(status,0,myset,rots)
C
C	Hand cursor
C
			Call hand_cursor
C
C	change the offsets while mouse is down
C
			Call getori(wleft,wbot)
C
C	get window space x,y
C
			xo = getval(CURSRX)-wleft
			yo = getval(CURSRY)-wbot
C
			Do while (getbut(device))
C
C	get window space x,y
C
				x = getval(CURSRX)-wleft
				y = getval(CURSRY)-wbot
C
C	compute deltas
C
				if (i .eq. 2) then
					ix = (xo-x)/wind2d(curwin).zoom2d
					iy = (yo-y)/wind2d(curwin).zoom2d
				Else if (i .eq. 3) then
					ix = (xo-x)/zoom3d
					iy = (yo-y)/zoom3d
				Endif
				If ((ix .ne. 0).or.(iy .ne. 0)) then 
C	cursor moved
					If (i .eq. 2) then
						call getsiz(tx,ty)
						Call bumpoffset(
     +						wind2d(curwin).offset2d,
     +						wind2d(curwin).zoom2d,
     +						ix,iy,imagesize,imagesize,
     +						tx,ty)              
						Call draw_image_i2(imagesize,
     +						imagesize,tx,ty,
     +						wind2d(curwin).offset2d,
     +						wind2d(curwin).zoom2d,
     +						%val(wind2d(curwin).im2d_ptr),
     +						%val(wind2d(curwin).tex2d_ptr),
     +						wind2d(curwin).texture_mode)
					Else if (i .eq. 3) then
						call getsiz(tx,ty)
						Call bumpoffset(offset3d,
     +						zoom3d,ix,iy,
     +						image3dsize,image3dsize,tx,ty)
						Call draw_image_i4(image3dsize,
     +						image3dsize,tx,ty,
     +						offset3d,zoom3d,%val(save_ptr),
     +						%val(zbufptr))
					Endif
					xo = x
					yo = y
C
					Call swapbu
				Endif
			Enddo
C
C	revert to old cursor
C
			Call arrow_cursor
C
			Call sel_window(winsave,0,myset,rots)
			Call qenter(REDRAW,winput)
			Goto 10
		Endif
	Endif
C
C	Get the modifiers 
C
	modifiers = 0
	depth = 0
	If (getbut(CTRLKE) .or. getbut(RIGHTC).or. 
     +		keyboard_query(KEYBOARD_CTRL)) modifiers = modifiers + 1
	If (getbut(LEFTSH) .or. getbut(RIGHTS).or.
     +		keyboard_query(KEYBOARD_SHIFT)) modifiers = modifiers + 2
     	If (getbut(LEFTAL) .or. getbut(RGHTAL).or. getbut(XKEY).or.
     +		keyboard_query(KEYBOARD_ALT)) modifiers = modifiers + 4
	IF (getbut(ZKEY)) depth = -999
C
C	Erase = ctrl only = 1
C	Backup = alt only = 4 | shift+alt = 6
C	Line = shift = 2
C	Trace = all other combos
C	3D_cursor = CTRL+ALT = 5 (one shot)
C
C	type *,"mod=",modifiers
	If (modifiers .eq. 5) cursor3d_track = 1  ! one shot cursor motion
C
C	Check for erasing of points 
C
	If ((device .eq. MOUSE3).and.(data .eq. 1).and.
     +		(modifiers .eq. 1)) then
C
C	cannot trace in the dialog window
C
		If (winput .eq. wid) goto 10
C
C	tempory window focus swap
C
		winsave = curwin
		Call find_window(winput,status)
		If (status .ge. 1) Call sel_window(status,0,myset,rots)
C
		Call getori(wleft,wbot)
C
C	force initial point
C
		xo = -999
		yo = -999
C
C	while mouse is down
C
		Do While (getbut(MOUSE3))
C
C	get window space x,y
C
			x = getval(CURSRX)-wleft
			y = getval(CURSRY)-wbot
			If ((xo .ne. x).or.(yo .ne. y)) Then
C
C	erase points while mouse button is down
C
				If (winput .eq. wid2) Then ! 3d window
       					Call erasepoints(num3dpts,w3dpts,
     +						num2dpts,w2dpts,-1,zoom3d,
     +						offset3d,x,y,cindex,i,myset)
				Else ! 2d window
       					Call erasepoints(num3dpts,w3dpts,
     +						num2dpts,w2dpts,
     +					wind2d(curwin).inum2d,
     +					wind2d(curwin).zoom2d,
     +					wind2d(curwin).offset2d,
     +						x,y,cindex,i,myset)
				Endif
C
C	draw the deleted points... (i of them)
C
				If (i .ne. 0) Then
				Call winset(wind2d(curwin).wid)
				call frontb(.true.)
				status = num2dpts + i
     				Call paintpoints(status,w2dpts,
     +					wind2d(curwin).inum2d,
     +					wind2d(curwin).zoom2d,
     +				wind2d(curwin).offset2d,myset)    
				call gfflush
				call frontb(.false.)
				Call winset(wid2)
				call frontb(.true.)
				status = num3dpts + i
     				Call paintpoints(status,w3dpts,
     +					-1,zoom3d,offset3d,myset)    
				call gfflush
				call frontb(.false.)
				If (winput .ne. 0) call winset(winput)
				Endif
C
C	reset the points
C
				xo = x
				yo = y
			Endif
			Call gflush
		Enddo
C
C	recompute the measurements
C
		Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +			myset,dialog,1)	
C
C	tempory window focus swap
C
		Call sel_window(winsave,0,myset,rots)
C
C	done...Redraw properly
C
		Call qenter(REDRAW,0)
		Goto 10
	Endif
C
C	Check for backup of traces
C
	If ((device .eq. MOUSE3).and.(data .eq. 1).and.
     +		((modifiers .eq. 4).or.(modifiers .eq. 6))) then
C
C	tempory window focus swap
C
		winsave = curwin
		Call find_window(winput,status)
		If (status .ge. 1) Call sel_window(status,0,myset,rots)
C
C	Sort the points into contig colors with current color on top
C
		Call pointsort(w2dpts,num2dpts,w3dpts,num3dpts,cindex)
C
C	remove points while mouse button is down
C
		Call get_pref_f(PREF_BACKUPRATE,pps)
		ms_per_point = 1000.0/pps
		i = 0
		Do while (getbut(MOUSE3))
C
			If ((num2dpts-I .gt. 0).and.
     +				(num3dpts-I .gt. 0).and.
     +				(w2dpts(num2dpts-I,4).eq.cindex)) then
				I = I + 1
C
C	color the removed point differently (BLACK or WHITE)
C
				If (w2dpts(num2dpts-I+1,4) .eq. 0) then
					w2dpts(num2dpts-I+1,4) = 7
					w3dpts(num3dpts-I+1,4) = 7
				Else
					w2dpts(num2dpts-I+1,4) = 0
					w3dpts(num3dpts-I+1,4) = 0
				Endif
			Endif
C
C	draw the deleted points...
C
			Call winset(wind2d(curwin).wid)
			call frontb(.true.)
     			Call paintpoints(num2dpts,w2dpts,
     +				wind2d(curwin).inum2d,
     +				wind2d(curwin).zoom2d,
     +				wind2d(curwin).offset2d,myset)    
			call gfflush
			call frontb(.false.)
			Call winset(wid2)
			call frontb(.true.)
     			Call paintpoints(num3dpts,w3dpts,
     +				-1,zoom3d,offset3d,myset)    
			call gfflush
			call frontb(.false.)
			If (winput .ne. 0) Call winset(winput)
			Call bv_sginap(ms_per_point)
		Enddo
C
C	actually delete the points
C
		num2dpts = num2dpts - I
		num3dpts = num3dpts - I
C
C	recompute the measurements
C
		Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +			myset,dialog,1)	
C
C	tempory window focus swap
C
		Call sel_window(winsave,0,myset,rots)
C
C	done...Redraw properly
C
		Call qenter(REDRAW,0)
		Goto 10
	Endif
C
C	check for tracing on either images
c
	if ((device .eq. MOUSE3).and.(data .eq. 1)) then
C
C	cannot trace in the dialog window
C
		If (winput .eq. wid) goto 10
C
C	tempory window focus swap
C
		winsave = curwin
		Call find_window(winput,status)
		If (status .ge. 1) Call sel_window(status,0,myset,rots)
C
		Call getori(wleft,wbot)
C
C	force initial point
C
		xo = -999
		yo = -999
C
C	while mouse is down
C
		Do While (getbut(MOUSE3))
C
C	get window space x,y
C
			x = getval(CURSRX)-wleft
			y = getval(CURSRY)-wbot
C
C	convert to image space
C
			if (winput .eq. wid2) then
C
C	convert window space x,y into image space by:
C		1) divide by zoom factor
C		2) subtract the offset
C
C	tracing in 3d window
C
				ix = (x/zoom3d)+offset3d(1)
				iy = (y/zoom3d)+offset3d(2)
			Else
C
C	tracing in 2d window
C
				ix = (x/wind2d(curwin).zoom2d)+
     +					wind2d(curwin).offset2d(1)
				iy = (y/wind2d(curwin).zoom2d)+
     +					wind2d(curwin).offset2d(2)
			Endif
C
C	whenever it moves
C
			if ((xo .ne. ix).or.(yo .ne. iy)) then !cursor moved
C
C	add points and paint
C
				if (winput .eq. wid2) then  ! 3d window
C
C	add the point (if need be) to the 2d list
C
					retvals(1) = depth
					i = num2dpts
					Call addpt(ix,iy,
     +						num2dpts,w2dpts,-1,cindex,
     +						myset,image3dsize,
     +						image3dsize,retvals)
					if (i .ne. num2dpts) then
C
C	Moving the 3D cursor???
C
	If (cursor3d_track .ne. 0) Then
		cursor3d_pos(1) = w2dpts(num2dpts,1)
		cursor3d_pos(2) = w2dpts(num2dpts,2)
		cursor3d_pos(3) = w2dpts(num2dpts,3)
		num2dpts = i
		If (winput .ne. 0) call winset(winput)
		status = 1
		goto 9876
	Endif
C
C	add to the 3d list as well
C
					num3dpts = num3dpts + 1
					if (num3dpts .gt. 4095) 
     +						num3dpts = 4095
					w3dpts(num3dpts,1) = retvals(1)
					w3dpts(num3dpts,2) = retvals(2)
					w3dpts(num3dpts,3) = retvals(3)
					w3dpts(num3dpts,4) = retvals(4)
C
C	Interpolate if shift key is held down
C
					if (modifiers .eq. 2) then
					i = 0
					   Call interp_pts(num2dpts,w2dpts,i)
					   Call interp_pts(num3dpts,w3dpts,i)
					Endif
					Call winset(wind2d(curwin).wid)
					call frontb(.true.)
     					Call paintpoints(num2dpts,w2dpts,
     +						wind2d(curwin).inum2d, 
     +						wind2d(curwin).zoom2d,
     +						wind2d(curwin).offset2d,myset)
					call gfflush
					call frontb(.false.)
					Call winset(wid2)
					call frontb(.true.)
     					Call paintpoints(num3dpts,w3dpts,
     +					     -1,zoom3d,offset3d,myset)
					call gfflush
					call frontb(.false.)
					Endif

				Else ! 2d window
					i = num3dpts
C
C	add the point (if need be) to the 3d list
C
					retvals(1) = depth
					Call addpt(ix,iy,
     +						num3dpts,w3dpts,
     +					    slicetoz(wind2d(curwin).inum2d),
     +						cindex,myset,image3dsize,
     +						image3dsize,retvals)
					if (i .ne. num3dpts) then
C
C	Moving the 3D cursor???
C
	If (cursor3d_track .ne. 0) Then
		cursor3d_pos(1) = retvals(1)
		cursor3d_pos(2) = retvals(2)
		cursor3d_pos(3) = retvals(3)
		num3dpts = i
		If (winput .ne. 0) call winset(winput)
		status = 1
		goto 9876
	Endif
C
C	add to the 2d list as well
C
					num2dpts = num2dpts + 1
					if (num2dpts .gt. 4095) 
     +						num2dpts = 4095
					w2dpts(num2dpts,1) = retvals(1)
					w2dpts(num2dpts,2) = retvals(2)
					w2dpts(num2dpts,3) = retvals(3)
					w2dpts(num2dpts,4) = retvals(4)
C
C	Interpolate if control key is held down
C
					if (getbut(CTRLKE) .or. 
     +			getbut(RIGHTC).or.keyboard_query(KEYBOARD_CTRL)) then
					i = 0
					Call interp_pts(num2dpts,w2dpts,i)
					Call interp_pts(num3dpts,w3dpts,i)
					Endif
					Call winset(wid2)
					call frontb(.true.)
    					Call paintpoints(num3dpts,w3dpts,
     +					     -1,zoom3d,offset3d,myset)
					call gfflush
					call frontb(.false.)
					Call winset(wind2d(curwin).wid)
					call frontb(.true.)
					Call paintpoints(num2dpts,w2dpts,
     +						wind2d(curwin).inum2d,
     +						wind2d(curwin).zoom2d,
     +						wind2d(curwin).offset2d,myset)
					call gfflush
					call frontb(.false.)
					Endif
				Endif
C
C	reset the points and the window
C
				xo = ix
				yo = iy
				If (winput .ne. 0) call winset(winput)
			Endif
C
C	redraw
C
			Call gflush
		Enddo
C		type *, "Mouse is up "
C
C	recompute the measurements
C
		Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +			myset,dialog,1)	
		status = 0
C		type *, "Measure done "
C
C	clean up
C
9876	Continue
C
C	tempory window focus swap
C
		Call sel_window(winsave,0,myset,rots)
C
C	Handle window locking
C
C		type *, "locking:",status
		If (status .eq. 1) Then
			cursor3d_track = 0   ! one shot tracking...
			Call pan_locked_windows(myset)
			Call setup_dialog(rots,dialog,wid,myset,0)
		Endif
C
		Call Qenter(REDRAW,0)
		Goto 10
	Endif
C
C	default
C
	Return
C
C	Setup return values...
C
C	Redraw
C
10	Continue
C	type *, "return 10"
	handled = 10
	Return
C
C	Through 7001
C
7001	Continue
C	type *, "return 7001"
	Handled = 7001
	Return

	End
C
C	Routine for handling MOUSE1 downs
C
	Subroutine handle_mouse1down(device,data,rots,dialog,
     +		cindex,handled)
C
	Implicit None
	
	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'voxeluif.inc'
	Include 'keyboard_inc.f'

	Include 'measure.inc'
	Include 'rotsdef.inc'
	Include 'voxel_globals.inc'
	Include 'windows.inc'
C
C	Parameters:
C
	Record	/rotsrec/	rots
	Record	/ditem/	dialog(100)
	Integer*4	handled
	Integer*4	device
	Integer*2	data
	Integer*4	cindex
C	
C 	Local:
C
	Integer*4	tx,ty,winsave
	Integer*4	i,status
C
C	Passed:
C		dialog,rots,myset,cindex,device,data
C
C	Common:
C		wid,wid2,winput,imagesize,image3dsize
C		grid1,gcol1,im_menu
C	Local:
	Handled = 0
C
C       look for right mouse (MOUSE1) down in image window
C
        if ((device.eq.MOUSE1).and.(data.eq.1)
     +          .and.(winput.eq.wid2)) then
C
C       mouse down in 3d image window
C
                i = dopup(im_menu)
                if (i .eq. -1) goto 10
                if (i .eq. 100) then
                   win_save = wid2
                else if (i .eq. 101) then 
                   call win2clip(wid2)
                endif
                if ((i .ge. KEYBOARD_SHIFT).and.
     +			(i .le. KEYBOARD_ALT)) then
                	Call keyboard_do_menu(i)
                Endif
                if ((i .ge. 1000).and.(i .lt. 2000)) then
                        grid1 = (float(i)-1000.0)/20.0
                        gcol1 = cindex
                        Call qenter(REDRAW,wid2)
                Endif
                if ((i .ge. 9200) .and. (i .le. 9210)) Then
                        Call Do_cursor3d(i,cindex)
                Endif
                if ((i .ge. 1) .and. (i .le. 5)) then
                        zoom3d = i
C
C       fix errors
C
                        call getsiz(tx,ty)
                        Call bumpoffset(offset3d,zoom3d,
     +                  0,0,
     +                  image3dsize,image3dsize,
     +                  tx,ty)
                        Call qenter(REDRAW,wid2)
                Endif
                goto 10
C
        Else if ((device.eq.MOUSE1).and.(data.eq.1)
     +          .and.(winput.ne.wid)) then
C
C       mouse down in 2d image window
C
                Call update_window_menu(winput)
                i = dopup(im_menu2)
                if (i .eq. -1) goto 10
                if (i .eq. 100) Then
C RJF - this was curwin???
			win_save = winput  
                Else if (i .eq. 101) then 
                   call win2clip(winput)
                Else if ((i .ge. KEYBOARD_SHIFT).and.
     +			(i .le. KEYBOARD_ALT)) Then
                	Call keyboard_do_menu(i)
                Else if ((i .ge. 9200) .and. (i .le. 9210)) Then
                        Call Do_cursor3d(i,cindex)
                Else if ((i .ge. 1000).and.(i .lt. 2000)) then
C
C       tempory window focus swap
C
                	winsave = curwin
                	Call find_window(winput,status)
                	If (status .ge. 1) Then
				Call sel_window(status,0,myset,rots)
			Endif
C
                	wind2d(curwin).grid = (float(i)-1000.0)/20.0
                	wind2d(curwin).gcol = cindex
                	Call qenter(REDRAW,wind2d(curwin).wid)
C
                	Call sel_window(winsave,0,myset,rots)
C
                Else if ((i .ge. 1) .and. (i .le. 5)) then
C
C       tempory window focus swap
C
                winsave = curwin
                Call find_window(winput,status)
                If (status .ge. 1) Call sel_window(status,0,myset,rots)
C
                        wind2d(curwin).zoom2d = i
C
C       fix errors
C
                        call getsiz(tx,ty)
                        Call bumpoffset(wind2d(curwin).offset2d,
     +                  	wind2d(curwin).zoom2d, 0,0,
     +                  	imagesize,imagesize,tx,ty)
                        Call qenter(REDRAW,wind2d(curwin).wid)
C
                Call sel_window(winsave,0,myset,rots)
C
C       Popup menu values >= 9000 (windows menu)
C
                Else if (i .eq. 9001) Then  ! select (wid)
                        Call find_window(winput,status)
                        If (status .ne. -1) Then
                                Call sel_window(status,1,myset,rots)
                                Call setup_dialog(rots,dialog,wid,
     +					myset,0)
                        Endif
                Else if (i .eq. 9002) Then  ! add new window
                        Call add_window(status,myset,rots)
                        Call setup_dialog(rots,dialog,wid,myset,0)
                Else if (i .eq. 9003) Then  ! remove this window
                        Call rem_window(winput,myset,rots)
                        Call setup_dialog(rots,dialog,wid,myset,0)
                Else if (i .ge. 9101) Then  ! select a new window
                        Call sel_window(i - 9100,1,myset,rots)
                        Call setup_dialog(rots,dialog,wid,myset,wid2)
C
C       Popup menu values >= 8000 (texture menu)
C
                Else if ((i .ge. 8000) .and. (i .le. 8002)) Then
                        Call find_window(winput,status)
                        If (status .ne. -1) Then
                                wind2d(status).texture_mode = i - 8000
                                Call dirty_window(status,1)
                                Call qenter(REDRAW,wind2d(status).wid)
                        Endif
C
C	locked 3D cursor toggle
C
		Else if (i .eq. 8003) Then
			Call find_window(winput,status)
			If (status .ne. -1) Then
				wind2d(status).cursor_locked = 1 -
     +					wind2d(status).cursor_locked
                                Call dirty_window(status,1)
                                Call qenter(REDRAW,wind2d(status).wid)
                        Endif
                Endif
                goto 10
	Endif
C
C	Default
C
	Return
C
C	Return values
C
10	Continue

	Handled = 10

	Return
	End
