	program resedit

	Implicit None
	
	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'resedituif.inc'

	Integer*4	status,wid,formwid,device,wx,wy,temp
	Integer*4	mx,my,i
	Integer*2	data
	Record /ditem/  work(100)
	Record /ditem/  subdlg(50)
	Character*255	tstr
C
C	menus
C
	Integer*4	color_menu,font_menu,grid_menu,type_menu
	common	/rmenu/color_menu,font_menu,grid_menu,type_menu
C
	Integer*2	pattern(16)
C
	Integer*4	focusid
C
	Record /ditem/  widdlg(100)
	Integer*4	nitems,selitem
	Common	/rdata/widdlg,nitems,selitem
C
	data	pattern/$AAAA,$0000,$5555,$0000,
     +			$AAAA,$0000,$5555,$0000,
     +			$AAAA,$0000,$5555,$0000,
     +			$AAAA,$0000,$5555,$0000/
C	
	Call prefsi(560,255)
	wid=winope("Resedit",7)
	Call RGBmod
	Call double
	Call gconfi
	Call minsiz(20,20)
	formwid=winope("Template",8)
	Call RGBmod
	Call double
	Call gconfi
	Call defpat(1,16,pattern)
	focusid = formwid
	Call init_dlg(status)
	if (status .ne. 0) call exit(1)
	Call get_dlg_comp('resedituif.r',1,50,widdlg,status)
	if (status .ne. 0) call exit(1)
C
C menus
C
	color_menu = newpup()
	font_menu = newpup()
	grid_menu = newpup()
	type_menu = newpup()
C
	tstr = "Line %x1|Fill %x2|Invert %x3|Item %x4|Feedback %x5|"//
     +		"Scroll Bar %x6|Text %x7|Background %x8}"
	Call addtop(color_menu,tstr,index(tstr,"}")-1,0)
	tstr = "1 %x1|2 %x2|3 %x3|4 %x4|5 %x5|"//
     +		"6 %x6|7 %x7|8 %x8|9 %x9|10 %x10}"
	Call addtop(font_menu,tstr,index(tstr,"}")-1,0)
	tstr = "Line %x4|Box %x5|Text %x6|Edit %x7|ScrollBar %x8|"//
     +	   "Menu %x9|Checkbox %x10|Button %x11|NOP %x12|Radio %x13}"
	Call addtop(type_menu,tstr,index(tstr,"}")-1,0)
	tstr = "1 %x1|2 %x2|5 %x5|10 %x10|20 %x20|25 %x25|50 %x50}"
	Call addtop(grid_menu,tstr,index(tstr,"}")-1,0)
	widdlg(gridmenu).aux = grid_menu
	widdlg(typemenu).aux = type_menu
C
	Call qdevic(MOUSE1)
	Call qdevic(MOUSE2)
	Call qdevic(MOUSE3)
	Call qdevic(WINQUI)
	Call qdevic(INPTCH)
	Call qenter(REDRAW,wid)
	Call qenter(REDRAW,formwid)
C
C	initialize the work array
C
	work(1).dtype = DLG_HEADER
	work(1).aux = 0
	work(1).rect(3) = -1
	work(1).rect(4) = -1
	work(2).dtype = DLG_END
	selitem = 0
	nitems = 2
C
10	device=qread(data)
	if (device .eq. INPTCH) then
		focusid = data
		If (focusid .ne. 0) call winset(focusid)
		goto 10
	Endif
	if (device .eq. WINQUI) then
		Call exit(0)
	Endif
	if (device .eq. REDRAW) then
		temp = winget()
		If ((data .eq. wid).or.(data .eq. 0)) then
C
C	redraw the main window
C
			call winset(wid)
			Call reshap
			Call ortho2(0.0,560.0,0.0,255.0)
			Call draw_dlg(widdlg,status)
			if (status .ne. 0) call exit(1)
			if (selitem .ne. 0) then
C
C	paint the sub dialog
C
				Call draw_dlg(subdlg,status)
				if (status .ne. 0) call exit(1)
			Endif
			Call swapbu
		Endif
		If ((data .eq. formwid).or.(data .eq. 0)) then
C
C	redraw the form window
C
			call winset(formwid)
			call getsiz(wx,wy)
C
C	if the window size changed: inform and redraw wid
C
			if ((work(1).rect(3) .ne. wx) .or.
     +				(work(1).rect(4) .ne. wy))
     +				 Call qenter(REDRAW,wid)
			work(1).rect(1) = 0
			work(1).rect(2) = 0
			work(1).rect(3) = wx
			work(1).rect(4) = wy
			write(unit=widdlg(windowtext).text,300) wx,wy
300	Format("(",I4.4,",",I4.4,")")
			Call reshap
			Call ortho2(0.0,float(wx),0.0,float(wy))
			Call drawit(work)
C
C	label/number the current selection
C
			Call swapbu
		Endif
		if (temp .ne. 0) call winset(temp) 
		goto 10
	Endif
C
C	handle the dialog events
C
	If (focusid .eq. wid) then
		call winset(wid)
		Call handle_dlg(device,data,widdlg,status)
		if (status .ne. 0) call exit(1)
C
C	events in the main dialog 
C
		If (device .eq. DLG_BUTEVNT) then
			if (data .eq. quit) call exit(0)
		Endif
		if (selitem .ne. 0) then
			Call handle_dlg(device,data,subdlg,status)
			if (status .ne. 0) call exit(1)
		Endif
C
C	events in the sub dialog
C
		goto 10
	Else if (focusid .eq. formwid) then
C
C	get pointer position
C
		Call getsiz(wx,wy)
		mx = getval(MOUSEX) - wx
		my = getval(MOUSEY) - wy
C
		If (device .eq. MOUSE1) then
			call winset(formwid)
			Call handle_dlg(device,data,work,status)
			if (status .ne. 0) call exit(1)
C
C	dialog events in the formsdialog (IGNORED!!!)
C
		Else if (device .eq. MOUSE2) then
C
C	middle mouse (select an item)
C
			Call finditem(work,mx,my,i)
			If (i .ne. selitem) then
				selitem = i
				Call selectitem(work,subdlg,.true.)
				Call qenter(REDRAW,0)
				goto 10
			Endif
C
		Else if (device .eq. MOUSE3) then
C
C	left mouse (move or resize)
C
			If (selitem .eq. 0) goto 10
C
		   Do while (getbut(MOUSE3))
			mx = getval(MOUSEX) - wx
			my = getval(MOUSEY) - wy
			If (getbut(LEFTAL).or.getbut(RGHTAL)) then
C
C	resize
C
				work(selitem).rect(3) = mx
				work(selitem).rect(4) = my
C
			Else
C
C	move
C
				i = work(selitem).rect(3)
     +					- work(selitem).rect(1)
				work(selitem).rect(1) = mx
				work(selitem).rect(3) = mx + i
	
				i = work(selitem).rect(4)
     +					- work(selitem).rect(2)
				work(selitem).rect(2) = my
				work(selitem).rect(4) = my + i
C
			Endif
			Call selectitem(work,subdlg,.false.)
			Call drawit(work)
		   Enddo
		   Call qenter(REDRAW,0)
		Endif
		goto 10
	Else
		Call ringbe
		goto 10
	Endif
C
	goto 10
	End
C
C	routine to find an item via a click
C
	Subroutine	finditem(dialog,mx,my,i)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'

	Record /ditem/	dialog(*)
	Integer*4	mx,my,i,j,x1,x2,y1,y2

	Integer*4	fdg
	Parameter	(fdg = 3)
C
C	assume no go!!!
C
	i = 0

	j = 2   ! 1 is the header  (I hope)
10	If (dialog(j).dtype .eq. DLG_END) return
C
C	is the object one which has screen space?
C
		If (dialog(j).dtype .eq. DLG_COLOR) Then
			j = j + 1
			goto 10
		Endif
C
C	get the object rectangle
C
		x1 = dialog(j).rect(1) 
		x2 = dialog(j).rect(3) 
		If (x1 .gt. x2) then
			x2 = dialog(j).rect(1) 
			x1 = dialog(j).rect(3) 
		Endif	
		y1 = dialog(j).rect(2) 
		y2 = dialog(j).rect(4) 
		If (y1 .gt. y2) then
			y2 = dialog(j).rect(2) 
			y1 = dialog(j).rect(4) 
		Endif	
C
C	if the object is a line then fudge the hit box a bit
C
		If (dialog(j).dtype .eq. DLG_LINE) Then
			x1 = x1 - fdg
			x2 = x2 + fdg
			y1 = y1 - fdg
			y2 = y2 + fdg
		Endif
C
C	Is the point in the rect?
C
		If ((x1 .le. mx).and.(x2 .ge. mx).and.
     +			(y1 .le. my) .and.(y2 .ge. my)) then
			i = j
			Return
		Endif
	j = j + 1
	goto 10
C
	End
C
C	routine to draw the forms dialog
C
	Subroutine	drawit(dialog)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'resedituif.inc'

	Record /ditem/	dialog(*)
C
	Integer*4	status,i,p(4)
C
	Record /ditem/  widdlg(100)
	Integer*4	nitems,selitem
	Common	/rdata/widdlg,nitems,selitem
C
	Call draw_dlg(dialog,status)
C
C	highlight
C
	If ((selitem .ne. 0).and.(widdlg(xitemchk).aux .eq. 1)) then
		Do i=1,4
			p(i) = dialog(selitem).rect(i)
		Enddo
		Call cpack($00000080)
		Call bgnlin
		Call v2i(p(1))
		Call v2i(p(3))
		Call endlin
		i = p(1)
		p(1) = p(3)
		p(3) = i
		Call bgnlin
		Call v2i(p(1))
		Call v2i(p(3))
		Call endlin
	Endif
C
	Call swapbu
C
	Return
	End
C
C	initialize / fill out the current object
C
	Subroutine selectitem(work,subdlg,init)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'resedituif.inc'

	Record /ditem/	subdlg(*)
	Record /ditem/	work(*)
	Integer*4	status
	logical		init
C
	Record /ditem/  widdlg(100)
	Integer*4	nitems,selitem
	Common	/rdata/widdlg,nitems,selitem
C
	Integer*4	color_menu,font_menu,grid_menu,type_menu
	common	/rmenu/color_menu,font_menu,grid_menu,type_menu
C
	If (init) then
		If (work(selitem).dtype .eq. DLG_LINE) then
		Call get_dlg_comp('resedituif.r',2,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_BOX) then
		Call get_dlg_comp('resedituif.r',3,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_BUTTON) then
		Call get_dlg_comp('resedituif.r',9,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_CHECK) then
		Call get_dlg_comp('resedituif.r',8,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_TEXT) then
		Call get_dlg_comp('resedituif.r',4,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_EDIT) then
		Call get_dlg_comp('resedituif.r',5,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_SBAR) then
		Call get_dlg_comp('resedituif.r',6,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_MENU) then
		Call get_dlg_comp('resedituif.r',7,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_RADIO) then
		Call get_dlg_comp('resedituif.r',11,50,subdlg,status)
		Else if (work(selitem).dtype .eq. DLG_NOP) then
		Call get_dlg_comp('resedituif.r',10,50,subdlg,status)
		Endif
	Endif
	
	write(unit=widdlg(itemtext2).text,10) work(selitem).rect(1),
     +		work(selitem).rect(2), work(selitem).rect(3), 
     +		work(selitem).rect(4)
10	Format(I3.3,",",I3.3,"-",I3.3,",",I3.3)

	write(unit=widdlg(itemtext).text,20) selitem,nitems
20	Format("Item:",I3.3," of ",I3.3)


	return
	End
