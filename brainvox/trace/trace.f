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
C    MODULE NAME    : _Main   
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_trace
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This is the main driver for brainvox_TRACE...
C			It handles all of the events and calls the
C			appropriate routines...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     voxtarce_trace rgnfile {-s} shmemid
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      -s is the 512x512 option...
C                                                                               
C                                                                               
C    ====================================================================== 
	Program brainvox_trace

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'
	Include 'vt_arb_inc.f'
	Include 'tr_windows.inc'
	Include 'keyboard_inc.f'
	Include 'prefs.f'
	Include '../filer/filer_inc.inc'

	Real*4		area
	Integer*4	status,state   !state is 1-select 2-tracing
	Integer*4	back_top,roi_top  !indexes of the scrolling lists
	Integer*4	menu2d,notes_menu,ms_per_point,wtmp
	Integer*4	device,I,trace(4096,2),J,modifiers,bv_os
	Integer*2	data
	Character*200	cmd,str9
	Character*256	tempstr
	Integer*4	message(260),msize,templut(1024),h
	logical		error
	External bv_os
C
	Integer*4	xo,yo,xp,yp,xx,yy,ctab(3,8),usergb
	Integer*4	grid_color
	Real*4		grid_cm,pps
	common		/grid_stuff/grid_color,grid_cm
C
	Integer*4	num2dpts,w2dpts(4096,4)
C
	common		/fvox/num2dpts,w2dpts
C
C	xo,yo is the PIXEL (image coordinates) which is to be displayed
C	in the lower left hand corner of the image space...
C
C	define 8 colors (R,G,B) RGBCMYBW        
C
	data	ctab/255,0,0,
     +			0,255,0,
     +			0,0,255,
     +			255,255,0,
     +			0,255,255,
     +			255,0,255,
     +			255,255,255,
     +			0,0,0/
C
C	tracing color
C
	c(1)=255
	c(2)=0
	c(3)=0
C
C       Guess the system path
C
        Call getarg(0,tempstr)
        Call bv_set_app_name(tempstr)
C
C	no grids
C
	grid_cm = 0
C
C 	Backgrounds...color
C
	b(1)=0
	b(2)=255
	b(3)=0
	usergb = 1
C
C	default palette
C
	Call read_pal("_ilut ",palette,error)
	If (error) then
		Do i=1,256
			palette(i)=i-1
			palette(i+256)=i-1
			palette(i+512)=i-1
		Enddo
	Endif
	Call read_pal("_tlut ",palette,error)
	If (error) then
		Do i=1,256
			tpalette(i)=i-1
			tpalette(i+256)=i-1
			tpalette(i+512)=i-1
		Enddo
	Endif
C
C	default autotracelevel
C
	autolevel = 0
C
C	we don't know what type of images yet  (256=256x256 512=512x512)
C
	imagesize = 0
C
C	no 2d points from voxel yet...
C
	num2dpts = 0
C
C	in the default state with editing off
C
	state=1
	editstate=0
C
C	editstate        meaning
C	0		 edit not active
C	1		 looking for first point
C	2		 looking for second point
C	3		 looking for third point
C	5		 autotracemode
C
C	create 2d window menu
C
	win_save = -1
	i = newpup()
C
C       Grid menu rollover (number = 1000 + 2*mm)
C
        tempstr = "None %x1000|0.2 cm %x1004|0.5 cm %x1010|"//
     +                  "1 cm %x1020|2 cm %x1040}"
	Call addtop(i,tempstr,index(tempstr,"}")-1,0)
	tempstr = "3 cm %x1060|4 cm %x1080|5 cm %x1100|10 cm %x1200}"
	Call addtop(i,tempstr,index(tempstr,"}")-1,0)
C
C	Notes
C
	notes_menu = newpup()
	tempstr = "Probe = CTRL+LMB%d%x9800"
     +		//"|Backup = ALT+LMB or ALT+SHIFT+LMB or X+LMB%d%x9801"
     +		//"|Line = SHIFT+LMB%d%x9802"
     +		//"|Pan = SHIFT+RMB or MMB%d%x9803"
     +		//"|E=edit,A=auto,R=revert,N=new,S=save,C=close%d%x9804}"
	Call addtop(notes_menu,tempstr,index(tempstr,"}")-1,0);
C
C	Image menu
C
	menu2d = newpup()
C
        tempstr="Save image %x100}"
        Call addtop(menu2d,tempstr,index(tempstr,"}")-1,0)
        If (bv_os() .eq. 0) then
           tempstr="Copy image %x101}"
           Call addtop(menu2d,tempstr,index(tempstr,"}")-1,0)
        Endif
        tempstr = "Zoom 1 %x1|Zoom 2 %x2|Zoom 3 %x3}"
        Call addtop(menu2d,tempstr,index(tempstr,"}")-1,0)
        tempstr="Zoom 4 %x4|Zoom 5 %x5|Zoom 6 %x6|"
     +		//"Zoom 7 %x7|Zoom 8 %x8|Grid %m}"
        Call addtop(menu2d,tempstr,index(tempstr,"}")-1,i)
	Call keyboard_create_menu(menu2d)
	tempstr = "Help %m}"
	Call addtop(menu2d,tempstr,index(tempstr,"}")-1,notes_menu)
C
C	find   the images,traces open the window...
C
	Call tr_init(wid,wid2)
	current_window = winget()
	arb_wid = 0
C
C	wx,wy is  window size  (768 or 512)
C	set the initial LL pixel coords...
C
	If (wx .le. 512) then
		xo=1
		yo=1
	Else
		xo=65
		yo=65
	endif
C
C	set top of backgound list... and roi list
C
	back_top=1
	roi_top=1
	Call setlists(back_top,roi_top)
C
C	set max entries for the scroll bars
C
	state1(5).tlength=numrgns-1
	state1(8).tlength=numrgns-1
	state2(5).tlength=numrgns-1
C
C	read in the traces into dynamic memory (ignore previously loaded)
C
	cur_slice=1
	Call readall
C
C	get the current ones from  dynamic...
C
	Call Set_Image_Scroll_Limits
C
	Call cleareditstate(state)
C
C	setup timer for checking IPCs
C
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)
C
	Call qdevic(WINFRE)
	Call qdevic(WINTHA)
	Call qdevic(TABKEY)
C
C	Here we go!!!
C
10	device=qread(data)	! top of event loop
C
C	winquits are no longer allowed...
C
        if (device .eq. TIMER0) then
                msize = 260
                call check_ipc(msize,message)
                if (message(1) .eq. IPC_T_ILUT) then   
C
C	New form palette editor 2D Image LUT message
C
			Call unpack_palette(message(2),templut)
			Do i=1,256
				palette(i) = templut(i)
				palette(i+256) = templut(i+256)
				palette(i+512) = templut(i+512)
			Enddo
			Call qenter(REDRAW,wid2)
			Call send_ipc(1,IPC_OK)
C
                else if (message(1) .eq. IPC_T_TLUT) then   
C
C	New form palette editor Texture LUT message
C
			Call unpack_palette(message(2),templut)
			Do i=1,256
				tpalette(i) = templut(i)
				tpalette(i+256) = templut(i+256)
				tpalette(i+512) = templut(i+512)
			Enddo
			Call qenter(REDRAW,wid2)
			Call send_ipc(1,IPC_OK)
C
                else if (message(1) .eq. IPC_ILUTS) then   
C
C	Old form palette editor message
C	first clear the queue!!!
C
			Call send_ipc(1,IPC_OK)
			Call read_pal("_ilut ",palette,error)
C
C	bad palette read => grayscale
C
			If (error) then
				Do i=1,256
					palette(i)=i-1
					palette(i+256)=i-1
					palette(i+512)=i-1
				Enddo
			Endif
			Call setpalette
			Call qenter(REDRAW,wid2)
		else if (message(1) .eq. IPC_POINT) then
C
C	get the points from voxel
C
			Call vt_read_points(num2dpts,w2dpts)
			Call send_ipc(1,IPC_OK)
			Call qenter(REDRAW,wid2)
		else if (message(1) .eq. IPC_CLEAR) then
C
C	clear overlay points
C
			num2dpts = 0
			Call qenter(REDRAW,wid2)
C
		else if (message(1) .eq. IPC_T_ARBREPLY) then
C
C	Got ARB values from Voxel
C
			If (message(2) .eq. 1) Then
C
C	Grab the values (rx,ry,rz,D)
C
				new_settings.rots(1) = message(3)
				new_settings.rots(2) = message(4)
				new_settings.rots(3) = message(5)
				new_settings.base_slice = message(6)-256
				Call Update_Arb_Window(1)
			Else
C
C	Not in ARB mode
C
			Call bv_error("The current voxel 2D slice is|"//
     +				"not in arbitry mode}",0)

			Endif
			Call send_ipc(1,IPC_OK)
			Call qenter(REDRAW,wid)
C
		else if (message(1) .eq. IPC_2D_TEXFILL) Then
C
C	Texture volume operations
C
			Call do_texture_op(message(2),message(3),message(4))
			Call qenter(REDRAW,0)
			Call send_ipc(1,IPC_OK)
			Call send_ipc(1,IPC_TEXVOLUME)
			Call tr_read_images(cur_slice,PD,T2)
C
		else if (message(1) .eq. IPC_TEXVOLUME) Then
C
C	Peek at a message directed at (Voxel)
C	(reread the texure volume image)
C
			Call tr_read_images(cur_slice,PD,T2)
			Call qenter(REDRAW,0)
C
		else if (message(1) .eq. IPC_TRACE_PRIVATE) Then
C
C	Internal message that the arb_mode has changed
C
			If (message(2) .eq. IPC_T_MODECHANGE) Then
				Call setlists(back_top,roi_top)
				Call qenter(REDRAW,0)
				Call send_ipc(1,IPC_OK)
			Endif
C
C	Save current tracing images to disk
C
		else if (message(1) .eq. IPC_F_RETURN) Then
			If (message(2) .eq. FLR_TRACE_IMAGES) Then
				Call get_filename(cmd,message(3),i)
				If (i .ne. 0) Then
					Call Save_Slices(cmd(1:i)//char(0))
				Endif
				Call send_ipc(1,IPC_OK)
			Endif
C
		else if (message(1) .eq. IPC_QUIT) then
C
C	Exit the program
C
			Call winclo(wid)
			Call winclo(wid2)
C
     			Call vt_quit_shmem
C
		        Call bv_exit(0)
C
		Endif
                goto 10
        Endif
c
c	t2 pd toggle
c
	if ((device .eq. PAD0).and.(data .eq. 1)) then
		pd_l = .not. pd_l
		Call setpalette
		Call doimage(xo,yo,state,trace) 
		Goto 10
	endif
c
c	handle keypad scrolling...
c
	if (((device .eq. PAD2).or.(device .eq. PAD8).or.
     +		(device .eq. PAD6).or.(device .eq. PAD4)).and.
     +		(data .eq. 1)) then
		call bumpsliders(device,state,32/zoom,0)
		call dodlg(state)
		Call setbysliders(xo,yo,state,trace)
		goto 10
	Endif
C
C	change the zoom...
C
	If ((device .ge. F1KEY).and.(device .le. F12KEY).and.
     +		(data .eq. 1)) then
		zoom=device-F1KEY+1
		Call setbysliders(xo,yo,state,trace)
		goto 10
	Endif
C
C	the thinning key
C
	If ((device .eq. PADAST).and.(data .eq. 1).and.
     +		(state .eq. 2)) then
C
		Call tr_thinroi(trace)
		Call qenter(REDRAW,wid2)
 		goto 10
C
	Endif
C
C	handle REDRAW
C
	If (device .eq. REDRAW) then	! redraw screen
C
		wtmp = winget()
C
C	resend the color map...
C
		Call setpalette
		Call doimage(xo,yo,state,trace) 
C
C	Dialog window
C
		Call winset(wid)
		Call reshap
		h = 'ffffffff'X
		If (usergb .eq. 0) Call writem(h)
		Call dodlg(state)
C
C	Arb window?
C
		If (arb_wid .ne. 0) Then
			Call winset(arb_wid)
			Call reshap
			Call ortho2(0.0,320.0,0.0,155.0)
			Call DRAW_DLG(arb_uif,status)
			Call swapbu
		Endif

C		if (current_window .ne. 0) call winset(current_window)
C
		If (win_save .eq. wid2) then
			call save_rgb("trace_img_%%%.rgb ",1)
			win_save = -1
		Endif
C
		Call winset(wtmp)
C
		Goto 10
	Endif
	If (device .eq. INPTCH) then	! change in input focus
		current_window = data
		if (data .ne. 0) call winset(current_window)
		goto 10
	Endif
C
C	Popup menu in the image window...
C
        modifiers = 0
        if (getbut(LEFTSH) .or. getbut(RIGHTS).or.
     +          keyboard_query(KEYBOARD_SHIFT)) modifiers = 1
C
	If ((device .eq. MOUSE1).and.(data .eq. 1)
     + 		.and. (modifiers .ne. 1)
     +		.and. (current_window .eq. wid2)) then
C
C	do the popup
C
                i = dopup(menu2d)
                if (i .eq. -1) goto 10
                if (i .eq. 100) then
			win_save = wid2
                else if (i .eq. 101) then
			Call win2clip(wid2)
                else if ((i .ge. 1) .and. (i .le. 8)) then
                        zoom = i 
			Call setbysliders(xo,yo,state,trace)
		else if ((i .ge. KEYBOARD_SHIFT) .and. 
     +			(i .le. KEYBOARD_ALT)) then
			Call keyboard_do_menu(i)
		else if ((i .ge. 1000) .and. (i .lt. 2000)) then
			grid_cm = (float(i)-1000.0)/20.0
			grid_color = b(1) + (b(2)*256) + (b(3)*65536)
			Call qenter(REDRAW,wid2)
		Endif
		goto 10
	Endif
C
C	Pan the image window...
C
	If ( ((device .eq. MOUSE2).or.
     +	      ((device .eq. MOUSE1).and.(modifiers .eq. 1)) )
     +		.and.(data .eq. 1)
     +		.and. (current_window .eq. wid2)) then
C
C	while mouse is down...
C
		Call hand_cursor
C
C	get start point
C
		xx=getval(CURSRX)
		yy=getval(CURSRY)
C
C	if the cursor moves then redraw the image
C
		Do while (getbut(device))
			xp=getval(CURSRX)
			yp=getval(CURSRY)
			i = (xx-xp)/zoom
			j = (yy-yp)/zoom
			If ((i .ne. 0) .or. (j .ne. 0)) then
				call bumpsliders(MOUSE2,state,i,-j)
				Call setbysliders(xo,yo,state,trace)
				xx = xp
				yy = yp
			Endif
		Enddo
C
C	allow system to refresh the overlay planes
C
		Call arrow_cursor
C
		Call qenter(REDRAW,wid2)
C
		goto 10
	Endif
C
C	Handle left mouse down in image window...
C
	If ((state .eq. 2).and.
     +		(((device .eq. TABKEY).or.(device .eq. MOUSE3))
     +		.and.(data .eq. 1))
     +		.and. (current_window .eq. wid2)) then
C
C	if alt key is down remove points ...
C
		Call get_pref_f(PREF_BACKUPRATE,pps)
		ms_per_point = 1000.0/pps
		if (getbut(LEFTAL).or.getbut(RGHTAL).or.
     +                  getbut(XKEY).or.
     +			keyboard_query(KEYBOARD_ALT)) then
			Call cleareditstate(state)
C
C	remove points while the button is down...
C
			i = trace(1,1)
			do while  (getbut(MOUSE3).or.getbut(TABKEY))
				if (i .gt. 0) then
					i = i - 1
					Call drawtraces(.true.,trace,-1-i,
     +						xo,yo,.true.)
				Endif
				Call gflush
				Call bv_sginap(ms_per_point)
			Enddo
			trace(1,1)=i
			Call qenter(REDRAW,wid2)
			goto 10
		Endif
C
C	if ctrl key is down revert to PIXVAL like activity...
C
		if (getbut(CTRLKE).or.getbut(LEFTCT).or.
     +			keyboard_query(KEYBOARD_CTRL)) then
C
C	get cursor...
C
			i= 0
			do while  (getbut(MOUSE3).or.getbut(TABKEY))
				xp=getval(CURSRX)
				yp=getval(CURSRY)
				Call getori(xx,yy)
				xp=xp-xx
				yp=yp-yy
C
C	in range??? (or off screen)
C
		If ((xp .gt. wx).or.(xp .lt.  0).or.(yp.gt. wy).or.
     +			(yp .lt. 0)) goto  10
C
C	convert to image space...
C
				xp=(xp/zoom)+xo-1
				yp=(yp/zoom)+yo-1
				if (xp+yp*512 .ne. I) then
					I=xp+yp*512
C			

C
C	get the pixel value
C
					if (xp .le. 0) xp = 1
					if (yp .le. 0) yp = 1
					if (xp .gt. 512) xp = 512
					if (yp .gt. 512) yp = 512
					if (pd_l) then
						j=pd(xp+(yp-1)*512)-256
					else
						j=t2(xp+(yp-1)*512)-256
					endif
C
C	generate the string
C
					write(str9,623) xp,yp,j
623	Format('X=',I3.3,' Y=',I3.3,' V=',I3.3)
					call writestr(str9,17,1)
				endif
			enddo
			goto 10
		endif
C
C	normal leftmouse activity (no keyboard modifiers)
C
C
C	check if EDITSTATE<>0
C
		if (editstate .eq. 0) then
C
C	tracing...Until close or mouseup
C
			do while  (getbut(MOUSE3).or.getbut(TABKEY))
				xp=getval(CURSRX)
				yp=getval(CURSRY)
				Call getori(xx,yy)
				xp=xp-xx
				yp=yp-yy
C
C	in range??? (or off screen)
C
		If ((xp .gt. wx).or.(xp .lt.  0).or.(yp.gt. wy).or.
     +			(yp .lt. 0)) goto  10
C
C	add the point on...
C	convert to image space...
C
				xp=(xp/zoom)+xo-exact_tracing
				yp=(yp/zoom)+yo-exact_tracing
				Call  addline(trace,xp,yp)
C
C	check for auto closing...
C
				If (trace(1,1) .gt. 10) then
					I=trace(1,1)+1
			If ((abs(trace(2,1)-trace(I,1)) .lt. 2) .and.
     +				(abs(trace(2,2)-trace(I,2)) .lt. 2)) then
C
C	close by adding the first point...
C
						xp=trace(2,1)
						yp=trace(2,2)
						Call addline(trace,xp,yp)
				Call drawtraces(.true.,trace,1,xo,yo,.true.)
C
C	ring the bell
C
						Call ringbe
						goto 10
					Endif
				Endif
C
C	draw the new segment
C
				Call drawtraces(.true.,trace,1,xo,yo,.true.)
				Call gflush
	   		Enddo
	   		goto 10
		Else
C
C	selecting points during edit trace..
C
			xp=getval(CURSRX)
			yp=getval(CURSRY)
			Call getori(xx,yy)
			xp=xp-xx
			yp=yp-yy
		If ((xp .gt. wx).or.(xp .lt.  0).or.(yp.gt. wy).or.
     +			(yp .lt. 0)) goto  10
C	
C	point is selected and onscreen
C
			xp=(xp/zoom)+xo
			yp=(yp/zoom)+yo
C
C	autotrace???
C
			If (editstate .eq. 5) then
				if (pd_l) then
				Call autotrace(pd,512,autolevel,trace,xp,yp)
				Else
				Call autotrace(t2,512,autolevel,trace,xp,yp)
				Endif
				Call qenter(REDRAW,wid2)
				Call cleareditstate(state)
				goto 10
			Endif
C
C	just selecting edit points
C
			edit(editstate,1)=xp
			edit(editstate,2)=yp
			editstate=editstate+1
C
C	bump editstate
C
			write(state2(12).text,678) editstate
678	Format('Edit point ',I1)
			Call dodlg(state)
			Call drawtraces(.true.,trace,1,xo,yo,.true.)
			Call gflush
C
C	all three points are defined???
C
			If (editstate .eq. 4) then
C
C	just do it...
C
C
C	perform the edit
C
				Call doedit(trace)
				Call cleareditstate(state)
				Call qenter(REDRAW,wid2)
				call ringbe
			Endif
		Endif
		goto 10
	Endif
C
C	try dialog interaction
C
1000	status = 0
	If ((state .eq. 1).and.(current_window .eq. wid)) then
		Call HANDLE_DLG(device,data,state1,status) 
	Else if ((state .eq. 2).and.(current_window .eq. wid)) then
		Call HANDLE_DLG(device,data,state2,status) 
	Endif
	If ((arb_wid .ne. 0) .and. (current_window .eq. arb_wid)) Then
		Call HANDLE_DLG(device,data,arb_uif,status) 
	Endif
	If (status .ne. 0) Call trace_exit(wid,wid2)
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	Translate hot keys...
C
	If ((device .eq. KEYBD).and.(state .eq. 2)) Then
		Call HotKeys(device,data)
	Endif
C
C	Handle SGIUIF events
C
	If ((current_window .eq. arb_wid) .and. (arb_wid .ne. 0)) Then
		Call Handle_Arb_Window(device,data,state)
		Goto 10
	Endif
C
C	handle popup menus...
C
	If (device .eq. DLG_POPEVNT) then
C
	   If (data .eq. 37) then
C
C	color selection
C
		If (state .eq. 1) then
			b(1)=ctab(1,state1(data).tpos)
			b(2)=ctab(2,state1(data).tpos)
			b(3)=ctab(3,state1(data).tpos)
		Else
			b(1)=ctab(1,state2(data).tpos)
			b(2)=ctab(2,state2(data).tpos)
			b(3)=ctab(3,state2(data).tpos)
		Endif
		Call qenter(REDRAW,wid2)
C
	   Else if (data .eq. 13) then
C
		c(1)=ctab(1,state2(data).tpos)
		c(2)=ctab(2,state2(data).tpos)
		c(3)=ctab(3,state2(data).tpos)
		Call qenter(REDRAW,wid2)
C
C	Copy a trace
C
	   Else if (data .eq. 34) then
C
		I = state2(data).tpos
		Do j=1,rois(i,1,1)+1
			trace(j,1) = rois(i,j,1)
			trace(j,2) = rois(i,j,2)
		Enddo
		Call qenter(REDRAW,wid2)
		goto 10
C
	Else if (data .eq. 39) then
C
C       if a menu selection was made close to an ROI
C
		if (editstate .ne. 0) then
			Call cleareditstate(state)
	   	Endif
                i = state2(data).tpos
                if (i .ne. -1) then
                    call ringbe
                    Call closehemi(trace,i,error)
                    call drawtraces(.true.,trace,1,xo,yo,.true.)
                Endif
		goto 10
C
	   Else if (data .eq. 38) then
C
		If (state .eq. 1) then
			I=state1(data).tpos
		Else
			I=state2(data).tpos
		Endif
		If ((I .eq. 1).or.(I .eq. 2)) then 
C
C	Traces on/off
C
			Do J=1,numrgns 
				If (I .eq. 1) then
					background(J)=.true.
				Else
					background(J)=.false.
				Endif
			Enddo
			Call setlists(back_top,roi_top)
			Call dodlg(state)
			Call qenter(REDRAW,wid2)
C
		Else if (I .eq. 3) then
C
			Call tr_thinroi(trace)
			Call qenter(REDRAW,wid2)
C
		Else if ((I .eq. 6).or.(I .eq. 7)) then
C
C	Auto-split hemis (preview=0)(real=1)
C
			Call split_hemis(i-6)
			Call qenter(REDRAW,wid2)
C
		Else if (I .eq. 4) then
C
			exact_tracing = 1-exact_tracing
			If (exact_tracing .eq. 1) Then
				Call change(state1(data).aux,3,
     +					"Exact Tracing %I%x4",19,0)
				Call change(state2(data).aux,3,
     +					"Exact Tracing %I%x4",19,0)
			Else
				Call change(state1(data).aux,3,
     +					"Exact Tracing %i%x4",19,0)
				Call change(state2(data).aux,3,
     +					"Exact Tracing %i%x4",19,0)
			Endif
			Call qenter(REDRAW,wid2)
C
		Else If (I .gt. 10) then
C
C	Do Gamma...
C
C			Call brainvox_gamma(float(I)/100.0)
			write(*,*) "Brainvox gamma no longer supported"
C
C			write(cmd,755) float(I)/100.0
C755	Format('brainvox_gamma ',F6.2)
C			Call system(cmd)
C
		Else If (I .eq. 5) Then
C
C	Open arb window
C
			Call Set_Arb_Window(-1)
C
		Else If (I .eq. 8) Then
C
C	File selection widget -> save all trace images...
C
756	Format("brainvox_filer ",I," ",I," ",A,
     +			" . -noret -dir -hide -f ",A," &")
			write(cmd,756) shm_data,FLR_TRACE_IMAGES,
     +				'"Select dir to save slices"',
     +				'"*"'
			Call bv_system_path(cmd)
		Endif
C
           Endif
C
C	set the map indexes
C
	   Call setpalette
	   goto 10
C
	Endif
C
C	someone clicked on clickable text...
C
	If (device .eq.  DLG_TXTEVNT) Then
		If (data .ge. 29) then
C
C	find the roiname selected
C
			cur_roi=data-29+roi_top
			if (cur_roi .gt. numrgns) then
				cur_roi=1
				goto 10
			Endif
			Do I=1,rois(cur_roi,1,1)+1
				trace(I,1)=rois(cur_roi,I,1)
				trace(I,2)=rois(cur_roi,I,2)
			Enddo
			state2(12).text='Tracing'
			state2(12).tlength=7
			state2(36).dtype = DLG_NOP
			call swapstates(state)
			Call setlists(back_top,roi_top)
			Call dodlg(state)
C			Call drawtraces(.true.,trace,-1,xo,yo,.true.)
C			Call drawtraces(.true.,trace,1,xo,yo,.true.)
			Call qenter(REDRAW,wid2)
			Goto 10
		Else
C
C	toggle a background trace
C
			if (data-21+back_top .gt. numrgns) then
				goto 10
			Endif
			background(data-21+back_top)= .not.
     +				background(data-21+back_top)
			Call setlists(back_top,roi_top)
			Call dodlg(state)
			Call qenter(REDRAW,wid2)
			Goto 10
		Endif
	Endif
C
C	handle scroll bars..
C
	If (device .eq. DLG_BAREVNT) then
		If (data .eq. 9) then
C
C	slice number list has scrolled
C
			I=(state1(9).aux*skip)+1
			If (I .eq. cur_slice) goto 10
			cur_slice=I
			Call Set_Image_Scroll_Limits
			Call qenter(REDRAW,wid2)
			Goto 10
C
		Elseif (data .eq. 5) then
C
C	background list has scrolled
C
			If (state .eq. 1) then
				back_top=state1(5).aux+1
			Else
				back_top=state2(5).aux+1
			Endif
			Call setlists(back_top,roi_top)
			Call dodlg(state)
			Goto 10
		Elseif (data .eq. 8) then
C
C	roilist has scrolled
C
			roi_top=state1(8).aux+1
			Call setlists(back_top,roi_top)
			Call dodlg(state)
C
			Goto 10
C
		Else if ((data .eq. 3).or.(data .eq. 4)) then
C
C	the sliders have been used to scroll the image...
C
			Call setbysliders(xo,yo,state,trace)
			goto 10
C
		Else if (data .eq. 36) then
C
C	autolevel slider
C
			autolevel = state2(36).aux
			Call qenter(REDRAW,wid2)
			goto 10
		Endif
	Endif
C
C	handle buttons...
C
	If (device .eq. DLG_BUTEVNT) then
		If (data .eq. 6) then
C
C	T2
C
			pd_l = .false.
			Call setpalette
			Call doimage(xo,yo,state,trace)    
			Goto 10
C
		Else if (data .eq. 7) then
C
C	PD
C
			pd_l = .true. 
			Call setpalette
			Call doimage(xo,yo,state,trace)    
			Goto 10
C
		Else if (data .eq. 8) then
C
C	EDit
C
			If (trace(1,1) .lt. 20) then
				editstate=0
				state2(12).text='Need more pts to edit'
				state2(12).tlength=21
				Call setpalette
				Call ringbe
			Else
				state2(36).dtype = DLG_NOP
				Call setpalette
				editstate=1
				state2(12).text='Edit point 1'
				state2(12).tlength=12
C
C	do a close first...
C
		           If (trace(1,1) .gt. 3) then
C
C	check if already closed...
C
				i=trace(1,1)+1
				if ((trace(2,1) .eq. trace(I,1)).and.
     +				   (trace(2,2) .eq. trace(I,2))) then
					Call dodlg(state)
					goto 10
				Endif
C
C	loop back...
C
				xp=trace(2,1)
				yp=trace(2,2)
				Call addline(trace,xp,yp)
				Call drawtraces(.true.,trace,1,xo,yo,.true.)
		   	   Endif
			Endif
C
			Call dodlg(state)
			goto 10
C
		Else if (data .eq. 9) then
C
C	close
C
		   if (editstate .ne. 0) then
			Call cleareditstate(state)
	   	   Endif
C
C       if CTRL key is down and user selected close then CLOSE TO ROI
C   RJF - moved to its own popup menu...
C
C		   If (getbut(CTRLKE).or.getbut(LEFTCT).or.
C    +			keyboard_query(KEYBOARD_CTRL)) then
C
C       if a menu selection was made close to an ROI
C
C                       i = dopup(state2(34).aux)
C                       if (i .ne. -1) then
C                               call ringbe
C                               Call closehemi(trace,i,error)
C                           call drawtraces(.true.,trace,1,xo,yo,.true.)
C                       Endif
C	   	   Endif
		   If (trace(1,1) .gt. 3) then
			call ringbe
C
C	check if already closed...
C
			i=trace(1,1)+1
			if ((trace(2,1) .ne. trace(I,1)).or.
     +			   (trace(2,2) .ne. trace(I,2))) then 
C
C	loop back...
C
				xp=trace(2,1)
				yp=trace(2,2)
				Call addline(trace,xp,yp)
				Call drawtraces(.true.,trace,1,xo,yo,.true.)
			Endif
		   Endif
		   Call calcarea(trace,area)
		   write(str9,625) area*
     +			(((float(imagesize)/512.0)*interpixel)**2.0)
625	Format("Area:",F8.1," mm2")
		   call writestr(str9,17,1)
		   goto 10
C
		Else if (data .eq. 10) then
C
C	backup
C
		   	if (editstate .ne. 0) then
				Call cleareditstate(state)
	   	   	Endif
			if (trace(1,1) .gt. 0) then
				trace(1,1)=trace(1,1)-1
c				Call drawtraces(.true.,trace,-1,xo,yo,.true.)
c				Call drawtraces(.true.,trace,1,xo,yo,.true.)
				Call qenter(REDRAW,wid2)
			Endif
			goto 10
C
		Else if (data .eq. 11) then
C
C	Revert to saved...
C
			Call cleareditstate(state)
			trace(1,1)=0
			call swapstates(state)
c			Call drawtraces(.true.,trace,-1,xo,yo,.true.)
c			Call drawtraces(.false.,trace,-1,xo,yo,.true.)
c			Call drawtraces(.false.,trace,1,xo,yo,.true.)
			Call qenter(REDRAW,wid2)
			Call dodlg(state)     
			Goto 10
C
		Else if (data .eq. 14) then
C
C	New   
C
			Call cleareditstate(state)
			trace(1,1)=0
C			Call drawtraces(.true.,trace,-1,xo,yo,.true.)
			Call qenter(REDRAW,wid2)
			Goto 10
C
		Else if (data .eq. 15) then
C
C	save   
C
			Call cleareditstate(state)
			rois(cur_roi,1,1)=trace(1,1)
			Do I=2,trace(1,1)+1
				rois(cur_roi,I,1)=trace(I,1)
				rois(cur_roi,I,2)=trace(I,2)
			Enddo
			trace(1,1)=0
			Call tr_write_roi(cur_roi)
			call swapstates(state)
C			Call drawtraces(.true.,trace,-1,xo,yo,.true.)
C			Call drawtraces(.false.,trace,-1,xo,yo,.true.)
C			Call drawtraces(.false.,trace,1,xo,yo,.true.)
			Call qenter(REDRAW,wid2)
			Call dodlg(state)     
			Goto 10
C
		Else if (data .eq. 39) then
C
C	Quit   
C
			Call trace_exit(wid,wid2)
C
		Else if (data .eq. 35) then
C
C	autotrace button
C
			if (editstate .eq. 5) then
C
C	aborted autotrace
C
				Call cleareditstate(state)
				goto 10
			Endif
C
C	starting autotrace
C
			editstate = 5     
			state2(36).dtype = DLG_SBAR
			Call dodlg(state)
C			Call setpalette
			Call qenter(REDRAW,wid2)
			goto 10
C
		Endif
	Endif

	Goto 10
C
C	see listing of brainvox.res for the definition of the dialog
C	objects... (And their all important numbering...)
C
	End
C
	subroutine	doimage(x,y,state,trace)
C
C	redraw the image in the forground... (no flipping)
C
	implicit none

	include 'fgl.h'
	include 'fdevice.h'
	include 'sgiuif.h'
	include '../libs/vt_inc.f'
	include 'tr_inc.f'
	Include 'tr_windows.inc'

	Integer*4	x,y,state,trace(4096,2),i,wtmp

	wtmp = winget()
C
	call winset(wid2)
	Call reshap
	Call check_size(x,y,state,trace)
	Call ortho2(0.0,float(wx-1),0.0,float(wy-1))
	Call tr_draw_image(x,y)
	Call drawtraces(.false.,trace,1,x,y,.false.)
	If (state .eq. 2) Call drawtraces(.true.,trace,1,x,y,.false.)
	Call swapbu
C
	Call winset(wtmp)

	Return
	End
C
C	Routine to handle a change in size...
C
	Subroutine check_size(xo,yo,state,trace)
	
	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'

	Integer*4	x,y
	Integer*4       xo,yo,state,trace(4096,2)
	
	Call getsiz(x,y)
	If ((x.ne.wx).or.(y.ne.wy)) Then
		wx = x
		wy = y
C
C	adjust zoom...
C
		x = 0
		Do while (512*zoom .lt. wx) 
			zoom = zoom + 1
			x = 1
		Enddo
		Do while (512*zoom .lt. wy) 
			zoom = zoom + 1
			x = 1
		Enddo
		Call setbysliders(xo,yo,state,trace)
	Endif

	Return
	End
C
	subroutine	dodlg(state)
C
C	redraw the dialog
C
	implicit none
	include 'fgl.h'
	include 'fdevice.h'
	include 'sgiuif.h'
	include '../libs/vt_inc.f'
	include 'tr_inc.f'
	Include 'tr_windows.inc'

	Integer*4	state,status,wtmp
	Character*200	str9

	wtmp = winget()
	Call winset(wid)
	Call ortho2(0.0,512.0,0.0,250.0)
	if (state .eq. 1) then
	 	Call DRAW_DLG(state1,status)
	Else
	 	Call DRAW_DLG(state2,status)
	Endif
	If (editstate .eq. 5) Then
		write(str9,624) autolevel
		call writestr(str9,17,0)
	Endif
	Call swapbu
	Call winset(wtmp)

624	Format('Autotrace lev=',I3.3)

	Return
	End
C
C
C
	Subroutine	setpalette

	Implicit None

	include "fgl.h"
	include "fget.h"
	include "sgiuif.h"
	include '../libs/vt_inc.f'
	include "tr_inc.f"

	Integer*4	I


	If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then

	If (pd_l) Then
	Do I=1,256
C
C	if in autotrace mode and the graylevel is > auto then tint color
C
		If ((editstate .eq. 5) .and. (I-1 .ge. autolevel)) Then
		cpalette(i) = palette(i) +  0 +
     +			palette(i+512)*256*256
		Else
		cpalette(i) = palette(i) + palette(i+256)*256 +
     +			palette(i+512)*256*256
		Endif
	Enddo
	Else
	Do I=1,256
C
C	if in autotrace mode and the graylevel is > auto then tint color
C
		If ((editstate .eq. 5) .and. (I-1 .ge. autolevel)) Then
		cpalette(i) = tpalette(i) +  0 +
     +			tpalette(i+512)*256*256
		Else
		cpalette(i) = tpalette(i) + tpalette(i+256)*256 +
     +			tpalette(i+512)*256*256
		Endif
	Enddo
	Endif
	cpalette(257) = b(1) + 256*b(2)+ 256*256*b(3)
	cpalette(258) = c(1) + 256*c(2)+ 256*256*c(3)
	Else

	If (pd_l) Then
	Do I=1,256
C
C	if in autotrace mode and the graylevel is > auto then tint color
C
		If ((editstate .eq. 5) .and. (I-1 .ge. autolevel)) Then
		Call mapcol(I+255,palette(I),0,palette(I+512))
		Else
		Call mapcol(I+255,palette(I),palette(I+256),palette(I+512))
		Endif
	Enddo
	Else
	Do I=1,256
C
C	if in autotrace mode and the graylevel is > auto then tint color
C
		If ((editstate .eq. 5) .and. (I-1 .ge. autolevel)) Then
		Call mapcol(I+255,tpalette(I),0,tpalette(I+512))
		Else
		Call mapcol(I+255,tpalette(I),tpalette(I+256),tpalette(I+512))
		Endif
	Enddo
	Endif
	Do I=768,1023
		Call mapcol(I,b(1),b(2),b(3))
	Enddo
	Do I=1280,2047
		Call mapcol(I,c(1),c(2),c(3))
	Enddo
	Call set_cindex_table
	Call gflush
	Endif

	Return
	End
C
C	routine to reset the edit state
C
	Subroutine cleareditstate(state)

	Implicit None

	include 'sgiuif.h'
	include '../libs/vt_inc.f'
	include 'tr_inc.f'

	Integer*4	state

	editstate=0
	state2(12).text='Tracing'      
	state2(12).tlength=7
	state2(36).dtype = DLG_NOP
	Call dodlg(state)
	Call setpalette

	return
	End
C
C
C
	Subroutine writestr(string,i,backb)

	Implicit None
	Include "fgl.h"
	Include "fdevice.h"
	Include "sgiuif.h"
	include '../libs/vt_inc.f'
	Include "tr_inc.f"
	Include 'tr_windows.inc'

	Character*(*)	string
	Integer*4	I,backb,wtmp,h

C
C	clear the space and position the cursor...
C
	wtmp = winget()
	Call winset(wid)
	If (backb .ne. 0) Call frontb(.true.)
	h = '00545454'X
	Call cpack(h)     ! gray sgiuif background color
	Call rectfi(5,0+5,5+20*11,16+5)
	h = '00ffffff'X
	Call cpack(h)
	call cmov2i(7,1+5)
	call charst(string,i)
	If (backb .ne. 0) Then
		Call gfflush
		Call frontb(.false.)
	Endif
	call winset(wtmp)

	return
	End
