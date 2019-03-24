	Program brainvox_pals

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include '../filer/filer_inc.inc'
	Include 'palsuif.inc'
	Include 'pals.inc'

	Integer*4	leftx
	parameter	(leftx = 220)

	logical		error

	Integer*4	status,i,state,j
	Integer*4	luts(3,256,4)  !(image,voxel,texture)(index)(r,g,b,a)
	Integer*4	disp(3,4),edit(3,4)
	Integer*4	device
	Integer*4	wid,twid,x,y,xo,wleft,wbot,yo
	Integer*2	data
	Integer*2	rd,gd,bd
	Integer*4	hist(256),csum,t_pal(256)
	Integer*4	use_i_hist,use_t_data
	Integer*4	thist(256),mhist(256),mihist(256),tihist(256)
	Character*500	tstr,tstr2
	Record /ditem/	dialog(100)
C
	Integer*4	t_ilut,v_ilut,v_vlut,v_tlut,t_tlut
C
C	IPC defines
C
	integer*4	size,message(IPC_SHM_SIZE)
C
	integer*4	iargc,strlen

	external 	iargc,strlen
C
	pins(1) = 0
	pins(2) = 255
	color_model = 1
	tool = 1
	ccount = 0
	use_i_hist = 0
	use_t_data = 0
	scale_pins = 0
C
C	Initially, all luts are assumed to be up to date
C
	t_ilut = 0
	v_ilut = 0
	v_vlut = 0
	v_tlut = 0
	t_tlut = 0
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
C	print version and program ID
C
	If ((iargc() .lt. 1).or.(bv_test_option(OPTS_TESTING))) Then
	write(*, 1)version,com_date
1	Format(" Welcome to BRAINVOX_PALS version:",A15,/,
     +	       " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
	Endif
C
	if (iargc() .lt. 1) then
2		write(*, 17)
17	Format("This program must be launched from within brainvox.") 
		Call bv_exit(1)
	Endif
	call getarg(iargc(),tstr)
	read(unit=tstr,fmt=*,err=2) shm_data
C
C	I am NOT the shared memory owner!!! (there can be only one!!)
C
	shmem_owner = 0
	Call vt_init_shmem
C
C	read in the pals
C
	Do state=3,1,-1
		Call pal_io(IPC_READ,state,luts)
	Enddo
	state = 1
	Call lut2edit(luts,state,1,256)
C
C	init status
C
	Do I=1,4
C	Image  (alpha now as well)
		disp(1,i) = 1 
		edit(1,i) = 1
C	Voxel
		disp(2,i) = 1
		edit(2,i) = 1
C	Texture
		disp(3,i) = 1
		edit(3,i) = 1
	Enddo
	Call histogram(mhist,csum,images_addr)
	Do i=1,256
		mihist(i) = 0
		thist(i) = 0
		tihist(i) = 0
	Enddo
	Call sethist(hist,mhist,mihist,thist,tihist,use_i_hist,use_t_data)
C
C	get the window
C
	x = (getgde(GDXPMA)-480)
	y = (0)
	call prefpo(x,x+480,y,y+300)
	wid=winope("Pal:Image",9)
	Call doubleb
	Call RGBmod
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('palsuif.r',1,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
C	Menus
C
	i = newpup()
	tstr = "Image palette %I%r1%x1|Voxel palette %r1%x2}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
        tstr = "Texture palette %r1%l%x3|Autoscale histogram %i%x101}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Histogram of 2DImage %i%x100|Texture volume histogram %i%x102}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	dialog(3).aux = i
C
	i = newpup()
	tstr = "Red Green Blue %I%r2%x1|Hue Saturation Value %r2%x2}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	dialog(pal_m_rgb).aux = i
C
	i = newpup()
	tstr = "4 Levels %x1004|8 Levels %x1008|16 Levels %x1016}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "32 Levels %x1032|64 Levels %x1064}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)

	dialog(pal_m_tools).aux = newpup()
	tstr = "Freehand %I%r3%x1|Fiddle %r3%x2|Rotate %r3%x3%l|"//
     +		"Posterize %m}"
	Call addtop(dialog(pal_m_tools).aux,tstr,index(tstr,"}")-1,i)
C
	i = newpup()
	tstr = "1 bit %x2001|2 bits %x2002|3 bits %x2003|4 bits %x2004}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "5 bits %x2005|6 bits %x2006|7 bits %x2007}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
C
	tstr = "Bitslice %m}"
	Call addtop(dialog(pal_m_tools).aux,tstr,index(tstr,"}")-1,i)
C
	i = newpup()
	dialog(pal_m_options).aux = i
	tstr = "Linear interpolate %x1|Smooth %x2|Vertical flip %x3|"//
     +		"Horizontal flip %x4%l}"
	Call addtop(dialog(pal_m_options).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Save palette as...%x5|Save opacity as...%x6}"
	Call addtop(dialog(pal_m_options).aux,tstr,index(tstr,"}")-1,0)
	i = newpup()
	tstr = "Banded %x101|Blue-green-red-yellow %x102}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Blue-red-yellow %x103|Gamma %x104}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Grayscale %x105|Hotbody I %x106}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Hotbody II %x107|Ice %x108}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Light Hues %x109|Rich Hues %x110}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Smooth Rich Hues %x111|16 Color palette %x112}"
	Call addtop(i,tstr,index(tstr,"}")-1,0)
	tstr = "Load palette...%x7|Load opacity...%x8|Save histogram...%x9%l"
     +		//"|Fixed maps %m}"
	Call addtop(dialog(pal_m_options).aux,tstr,index(tstr,"}")-1,i)
C	Call setpup(dialog(pal_m_options).aux,6,PUPGRE)
C	Call setpup(dialog(pal_m_options).aux,8,PUPGRE)
C
	Call setup_dialog(state,disp,edit,luts,dialog)
C
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qdevic(WINQUI)
	Call qdevic(MOUSE2)
	Call qdevic(MOUSE3)
	Call qenter(REDRAW,wid)	! force a redraw
10	device=qread(data)	! top of event loop
C	If (device .eq. WINQUI) then	! end of program
C		goto 999    
C	Endif
	if (device .eq. TIMER0) then
		size = IPC_SHM_SIZE
		call check_ipc(size,message)
		If (message(1) .eq. IPC_QUIT) Then
			goto 999
		Else if (message(1) .eq. IPC_I_HISTO) Then
C
C	Histograms
C
			If (message(258) .eq. IPC_H_MRICUT) Then
				Do i=1,256
					mihist(i) = message(i+1)
				Enddo
			Else if (message(258) .eq. IPC_H_MRIVOL) Then
				Do i=1,256
					mhist(i) = message(i+1)
				Enddo
			Else if (message(258) .eq. IPC_H_TEXTURECUT) Then
				Do i=1,256
					tihist(i) = message(i+1)
				Enddo
			Else if (message(258) .eq. IPC_H_TEXTUREVOL) Then
				Do i=1,256
					thist(i) = message(i+1)
				Enddo
				Call histogram(thist,csum,texture_addr)
			Endif
			Call sethist(hist,mhist,mihist,thist,tihist,
     +				use_i_hist,use_t_data)
			Call send_ipc(1,IPC_OK)
			Call qenter(REDRAW,wid)	! force a redraw
C
		Else if (message(1) .eq. IPC_F_RETURN) Then
C
C	Filer info
C
			If ((message(2) .eq. FLR_PAL1_S_OPAC).or.
     +				(message(2) .eq. FLR_PAL2_S_OPAC).or.
     +				(message(2) .eq. FLR_PAL3_S_OPAC).or.
     +				(message(2) .eq. FLR_PAL1_S_PAL).or.
     +				(message(2) .eq. FLR_PAL2_S_PAL).or.
     +				(message(2) .eq. FLR_PAL3_S_PAL)) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
				Call named_pal_io(message(2),luts,tstr,i)
				Endif
				Call send_ipc(1,IPC_OK)
			Else if ((message(2) .eq. FLR_PAL1_L_OPAC).or.
     +				(message(2) .eq. FLR_PAL2_L_OPAC).or.
     +				(message(2) .eq. FLR_PAL3_L_OPAC).or.
     +				(message(2) .eq. FLR_PAL1_L_PAL).or.
     +				(message(2) .eq. FLR_PAL2_L_PAL).or.
     +				(message(2) .eq. FLR_PAL3_L_PAL)) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
				Call named_pal_io(message(2),luts,tstr,i)
				Call lut2edit(luts,state,1,256)
				Call dirty_pals(i,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
				Endif
				Call send_ipc(1,IPC_OK)
			Else if (message(2) .eq. FLR_PAL_HISTO) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
					Call save_hist(tstr,hist)
				Endif
				Call send_ipc(1,IPC_OK)
			Endif
C
		Else if (message(1) .eq. IPC_P_BITSLICE) Then
C
C	bitslicing (0,1 volume)(flags=bits|scaled*256)(min)(max)
C
			Call bitslice_palette(luts,message(2),
     +				iand(message(3),255),iand(message(3),256),
     +				message(4),message(5))
			Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
			Call qenter(REDRAW,wid)
			Call send_ipc(1,IPC_OK)
			
		Else if (message(1) .eq. IPC_OK) Then
C
C	The queue is free, so send out any pending messages we have
C
			If (t_ilut .ne. 0) Then
				Call send_palette(luts,IPC_T_ILUT)
				t_ilut = 0
			Else if (v_ilut .ne. 0) Then
				Call send_palette(luts,IPC_V_ILUT)
				v_ilut = 0
			Else if (v_vlut .ne. 0) Then
				Call send_palette(luts,IPC_V_VLUT)
				v_vlut = 0
			Else if (v_tlut .ne. 0) Then
				Call send_palette(luts,IPC_V_TLUT)
				v_tlut = 0
			Else if (t_tlut .ne. 0) Then
				Call send_palette(luts,IPC_T_TLUT)
				t_tlut = 0
			Endif
		Endif
		goto 10
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call update_pin_text(dialog,state,luts)
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) goto 999     
		Call draw_hist(hist,csum)
		Call draw_luts(state,disp,edit,luts)
		Call swapbu
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) goto 999    
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	handle mouse down
C
	If (((device .eq. MOUSE2).or.(device .eq. MOUSE1))
     +		.and.(data .eq. 1)) then
C
C	get the window position
C
		Call getori(wleft,wbot)
		xo=-1
C
C	get the position
C
		X = getval(MOUSEX)-wleft-(leftx+1)
C
C	is it viable??
C
		if ((x .lt. 0).or.(x .gt. 255)) goto 10
C
		j = 1
		if (abs(x-pins(1)) .gt. abs(x-pins(2))) j = 2
		Do while (getbut(device))
C
C	get the position
C
			X = getval(MOUSEX)-wleft-(leftx+1)
C
C	bracket values (0-255)
C
			X = min(X,255)
			X = max(0,X)
C
C	redraw
C
			If (X .ne. xo) then
				pins(j) = x
				Call update_pin_text(dialog,state,luts)
				Call DRAW_DLG(dialog,status)
				Call draw_hist(hist,csum)
     				Call draw_luts(state,disp,edit,luts)
				Call swapbu
			Endif
C
C	record last position
C
			xo = x
		Enddo
		If (pins(1) .gt. pins(2)) Then
			i = pins(1)
			pins(1) = pins(2)
			pins(2) = i
			Call qenter(REDRAW,wid)
		Endif
		goto 10
	Endif

	If ((device .eq. MOUSE3).and.(data .eq. 1)) then

		Call do_tool(state,luts,edit,disp,dialog,hist,csum)
		Call dirty_pals(state,v_ilut,t_ilut,
     +				v_vlut,v_tlut,t_tlut,wid)
		goto 10
	Endif
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
C
C	Try the click tools first
C
		i = -1
		If (data .eq. 5) then   !reset
			Call click_tool(4,luts,edit,state,i)
		Else If (data .eq. pal_b_copy) then   !copy
			Call click_tool(1,luts,edit,state,i)
		Else If (data .eq. pal_b_paste) then   !paste
			Call click_tool(2,luts,edit,state,i)
		Else If (data .eq. pal_b_mpaste) then   !mpaste
			Call click_tool(3,luts,edit,state,i)
		Endif
		If (i .eq. 1) Then
			Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
		Endif
		if (i .ne. -1) goto 10 
C
		If (data .eq. 6) then   !save
			Call pal_io(IPC_WRITE,state,luts)
			goto 10 
		Else If (data .eq. 7) then   !revert
			Call pal_io(IPC_READ,state,luts)
			Call lut2edit(luts,state,1,256)
			Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
			goto 10 
		Endif
		goto 10
	Endif
C
C	handle radio events
C
	if (device .eq. DLG_POPEVNT) then
		if (data .eq. 3) then  ! Lut selection
		    if (dialog(data).tpos .eq. 100) Then
			use_i_hist = 1 - use_i_hist
			Call sethist(hist,mhist,mihist,thist,tihist,
     +				use_i_hist,use_t_data)
		    Else if (dialog(data).tpos .eq. 101) Then
			scale_pins = 1 - scale_pins
		    Else if (dialog(data).tpos .eq. 102) Then
			use_t_data = 1 - use_t_data
			Call sethist(hist,mhist,mihist,thist,tihist,
     +				use_i_hist,use_t_data)
		    Else if (dialog(data).tpos .gt. 0) Then
		    	state=dialog(data).tpos
			If (state .eq. 1) Then
				Call wintit("Pal:Image",9)
			Else if (state .eq. 2) then
				Call wintit("Pal:Voxel",9)
			Else if (state .eq. 3) Then
				Call wintit("Pal:Texture",11)
			Endif
			Call lut2edit(luts,state,1,256)

C	Image now has alpha
C			If (state .eq. 1) Then
C			Call setpup(dialog(pal_m_options).aux,6,PUPGRE)
C			Call setpup(dialog(pal_m_options).aux,8,PUPGRE)
C			Else
C			Call setpup(dialog(pal_m_options).aux,6,PUPNON)
C			Call setpup(dialog(pal_m_options).aux,8,PUPNON)
C			Endif

		    Endif
		    Call setup_dialog(state,disp,edit,luts,dialog)
		Else if (data .eq. pal_m_rgb) then  ! mode selection
			color_model = dialog(data).tpos
			Call lut2edit(luts,state,1,256)
			Call setup_dialog(state,disp,edit,luts,dialog)
		Else if (data .eq. pal_m_tools) then  ! tool selection
			If (dialog(data).tpos .ge. 2000) Then
C
C	bitslice
C
				Call click_tool(dialog(data).tpos,luts,
     +					edit,state,i)
				Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
			Else if (dialog(data).tpos .ge. 1000) Then
C
C	posterize
C
				Call click_tool(dialog(data).tpos,luts,
     +					edit,state,i)
				Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
			Else 
C
C	new tool
C
				tool = dialog(data).tpos
				Call setup_dialog(state,disp,edit,luts,
     +					dialog)
			Endif
		Else if (data .eq. pal_m_options) then  ! options
			i = 0
			If (dialog(data).tpos .gt. 100) Then
			Call fixed_cmap(%val(dialog(data).tpos-100),
     +				t_pal)
			Do i=1,256
				luts(state,i,1) = iand(t_pal(i),255)
				luts(state,i,2) = iand(t_pal(i),256*255)
				luts(state,i,3) = iand(t_pal(i),256*256*255)
				luts(state,i,2) = luts(state,i,2)/256
				luts(state,i,3) = luts(state,i,3)/65536
			Enddo
			Call lut2edit(luts,state,1,256)
			i = 1
			Else if (dialog(data).tpos .eq. 1) Then !linear
				Call click_tool(5,luts,edit,state,i)
			Else if (dialog(data).tpos .eq. 2) Then !smooth
				Call click_tool(6,luts,edit,state,i)
			Else if (dialog(data).tpos .eq. 3) Then !vflip
				Call click_tool(7,luts,edit,state,i)
			Else if (dialog(data).tpos .eq. 4) Then !hflip
				Call click_tool(8,luts,edit,state,i)
C
C	Filer tools
C
100	Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A," &")
110	Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A,
     +		" -s ",A," &")
			Else if (dialog(data).tpos .eq. 5) Then !Save LUT
				i = FLR_PAL1_S_PAL
			tstr2 = '"Save 2D image palette as... (*.pal)"}'
				if (state .eq. 2) Then
					i = FLR_PAL2_S_PAL
					tstr2 = "voxel}"
			tstr2 = '"Save voxel palette as... (*.pal)"}'
				Endif
				if (state .eq. 3) Then
					i = FLR_PAL3_S_PAL
			tstr2 = '"Save texture palette as... (*.pal)"}'
				Endif
				write(tstr,110) shm_data,i,
     +				tstr2(1:index(tstr2,"}")-1),
     +				'"*.pal"',"untitled.pal"
				Call bv_system_path(tstr)
				i = 0
			Else if (dialog(data).tpos .eq. 6) Then !Save Opac
				i = FLR_PAL1_S_OPAC
			tstr2 = '"Save 2D image opacity as... (*.opac)"}'
				if (state .eq. 2) Then
					i = FLR_PAL2_S_OPAC
			tstr2 = '"Save voxel opacity as... (*.opac)"}'
				Endif
				if (state .eq. 3) Then
					i = FLR_PAL3_S_OPAC
			tstr2 = '"Save texture opacity as... (*.opac)"}'
				Endif
				write(tstr,110) shm_data,i,
     +				tstr2(1:index(tstr2,"}")-1),
     +				'"*.opac"',"untitled.opac"
				Call bv_system_path(tstr)
				i = 0
			Else if (dialog(data).tpos .eq. 7) Then !Load LUT
				i = FLR_PAL1_L_PAL
			tstr2 = '"Load 2D image palette... (*.pal)"}'
				if (state .eq. 2) Then
					i = FLR_PAL2_L_PAL
			tstr2 = '"Load voxel palette... (*.pal)"}'
				Endif
				if (state .eq. 3) Then
					i = FLR_PAL3_L_PAL
			tstr2 = '"Load texture palette... (*.pal)"}'
				Endif
				write(tstr,100) shm_data,i,
     +				tstr2(1:index(tstr2,"}")-1),
     +				'"*.pal"'
				Call bv_system_path(tstr)
				i = 0
			Else if (dialog(data).tpos .eq. 8) Then !Load Opac
				i = FLR_PAL1_L_OPAC
			tstr2 = '"Load 2D image opacity... (*.opac)"}'
				if (state .eq. 2) Then
					i = FLR_PAL2_L_OPAC
			tstr2 = '"Load voxel opacity... (*.opac)"}'
				Endif
				if (state .eq. 3) Then
					i = FLR_PAL3_L_OPAC
			tstr2 = '"Load texture opacity... (*.opac)"}'
				Endif
				write(tstr,100) shm_data,i,
     +				tstr2(1:index(tstr2,"}")-1),
     +				'"*.opac"'
				Call bv_system_path(tstr)
				i = 0
			Else if (dialog(data).tpos .eq. 9) Then !Save Histo
				i = FLR_PAL_HISTO
				tstr2 = '"Save histogram as... (*.txt)"}'
				write(tstr,110) shm_data,i,
     +				tstr2(1:index(tstr2,"}")-1),
     +				'"*.txt"',"untitled.txt"
				Call bv_system_path(tstr)
				i = 0
			Endif
			if (i .eq. 1) Then
				Call dirty_pals(state,v_ilut,t_ilut,
     +					v_vlut,v_tlut,t_tlut,wid)
			Endif
		Endif
		goto 10
	Endif
C
C	handle checkbox events
C
	if (device .eq. DLG_CHKEVNT) then
		if ((data .ge. 10).and.(data .le. 13)) then
			disp(state,data-9) = dialog(data).aux
			i=data-9
			if (disp(state,i) .eq. 0) edit(state,i) = 0
		Else if ((data .ge. 14).and.(data .le. 17)) then
			edit(state,data-13) = dialog(data).aux
			i=data-13
			if (edit(state,i) .eq. 1) disp(state,i) = 1
		Endif
		Call setup_dialog(state,disp,edit,luts,dialog)
		goto 10
	Endif
C
	Goto 10
C
C	quit routine  
C
999	continue
C
C	since we are a lower life form we should not send IPCs
C	cut the umbilical
C
	Call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)

1000	continue
C
	write(*,*)" "
	write(*,*)"Brainvox is not validated for use with this dataset."
	write(*,*)" "
C
        Call send_ipc(1,IPC_QUIT)
C
C       cut the umbilical
C
        Call vt_quit_shmem
C
	Call bv_exit(0)
C
	End
C
C	routine to fill manage dialogs
C
	subroutine setup_dialog(state,disp,edit,luts,dialog)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'palsuif.inc'
	Include 'pals.inc'
C
	Integer*4	state,i
	Integer*4	luts(3,256,4)  !(image,voxel)(index)(r,g,b,a)
	Integer*4	disp(3,4),edit(3,4)
	Character*200	tstr
C
	Integer*4	strlen

	Record /ditem/	dialog(*)

	external	strlen
C
C	next the checkboxes
C
	If (color_model .eq. 1) Then
		dialog(10).text = "Red  "
		dialog(11).text = "Green"
		dialog(12).text = "Blue "
		dialog(14).text = "Red  "
		dialog(15).text = "Green"
		dialog(16).text = "Blue "
	Else
		dialog(10).text = "Hue  "
		dialog(11).text = "Sat  "
		dialog(12).text = "Value"
		dialog(14).text = "Hue  "
		dialog(15).text = "Sat  "
		dialog(16).text = "Value"
	Endif
	do I=1,4
		dialog(9+i).aux = disp(state,i)
		dialog(13+i).aux = edit(state,i)
	Enddo
	dialog(13).dtype = DLG_CHECK 
	dialog(17).dtype = DLG_CHECK 

C	if (state .eq. 1) then
C		dialog(13).dtype = DLG_NOP
C		dialog(17).dtype = DLG_NOP
C	Endif

C
C	menu texts
C
	If (color_model .eq. 1) Then
		dialog(pal_m_rgb).text = "RGB Mode"
	Else
		dialog(pal_m_rgb).text = "HSV Mode"
	Endif
	dialog(pal_m_rgb).tlength = 8
	If (tool .eq. 1) Then
		dialog(pal_m_tools).text = "Freehand"
	Else if (tool .eq. 2) Then
		dialog(pal_m_tools).text = "Fiddle  "
	Else if (tool .eq. 3) Then
		dialog(pal_m_tools).text = "Rotate  "
	Endif
	dialog(pal_m_tools).tlength = 8
C
C	redraw
C
	Call qenter(REDRAW,0)
C
	Return
	End
C
	Subroutine	update_pin_text(dialog,state,luts)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'palsuif.inc'
	Include 'pals.inc'
C
	Integer*4	luts(3,256,4)  !(image,voxel)(index)(r,g,b,a)
	Character*200	tstr
	Integer*4	state
C
	Record /ditem/	dialog(*)
C
C	Pins info
C
	if (color_model .eq. 1) Then
	write(UNIT=tstr,FMT=10) pins(1),int(ed_pal(pins(1)+1,1)*255),
     +		int(ed_pal(pins(1)+1,2)*255),int(ed_pal(pins(1)+1,3)*255),
     +		int(ed_pal(pins(1)+1,4)*255)
	dialog(pal_t_pin1).text = tstr
	write(UNIT=tstr,FMT=10) pins(2),int(ed_pal(pins(2)+1,1)*255),
     +		int(ed_pal(pins(2)+1,2)*255),int(ed_pal(pins(2)+1,3)*255),
     +		int(ed_pal(pins(2)+1,4)*255)
	dialog(pal_t_pin2).text = tstr
	Else 
	write(UNIT=tstr,FMT=20) pins(1),int(ed_pal(pins(1)+1,1)*255),
     +		int(ed_pal(pins(1)+1,2)*255),int(ed_pal(pins(1)+1,3)*255),
     +		int(ed_pal(pins(1)+1,4)*255)
	dialog(pal_t_pin1).text = tstr
	write(UNIT=tstr,FMT=20) pins(2),int(ed_pal(pins(2)+1,1)*255),
     +		int(ed_pal(pins(2)+1,2)*255),int(ed_pal(pins(2)+1,3)*255),
     +		int(ed_pal(pins(2)+1,4)*255)
	dialog(pal_t_pin2).text = tstr
	Endif
10	Format(I3,":",I3,"R ",I3,"G ",I3,"B ",I3,"A")
20	Format(I3,":",I3,"H ",I3,"S ",I3,"V ",I3,"A")
	
	Return
	End
C
C	routine to draw the histogram
C
	subroutine	draw_hist(hist,csum)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'pals.inc'

	Integer*4	hist(256),csum

	Integer*4	i,j,p1(2),h
	real*4		mx

	Integer*4	leftx
	parameter	(leftx = 220)

	Character*200	t1
C
C	find max value (ignore 0 and 255)
C
	if (scale_pins .eq. 0) Then
		j = 2
		Do i=3,255
			if (hist(i) .gt. hist(j)) j = i
		Enddo
	Else
		j = pins(1)
		Do i=j,pins(2)
			if (hist(i) .gt. hist(j)) j = i
		Enddo
	Endif
	mx = hist(j)
	if (mx .le. 0) mx = 1
C
C	Draw it
C
	Call cpack(0)
C
	Call bgnlin
	Do i = 1,256
		p1(1) = i + leftx
		p1(2) = int((float(hist(i))/mx)*255.0)
		if (p1(2) .gt. 255) p1(2) = 255
		if (p1(2) .lt. 0) p1(2) = 0
		p1(2) = p1(2) + 5
		Call v2i(p1)
	Enddo
	Call endlin
C
C	Draw dataset ID if required
C
	If (getbut(INSERT)) then
		h = '00ffffff'X
		Call cpack(h)
		Call cmov2i(leftx+5,128)
		write(unit=t1,fmt=10) csum
10	Format("Dataset ID:",I10)
		Call charst(t1,21)
	Endif
C
	return
	End
C
C	routine to draw the lut
C
	subroutine	draw_luts(state,disp,edit,luts)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'pals.inc'

	Integer*4	leftx
	parameter	(leftx = 220)
C
	Integer*4	state,i,k
	Integer*4	luts(3,256,4)  !(image,voxel)(index)(r,g,b,a)
	Integer*4	disp(3,4),edit(3,4)
C
	Integer*4	p1(2),p2(2),h
C
C	draw the bounding box
C
	Call cpack(0)  ! black
	Call recti(leftx,5,leftx+257,261)
	Call recti(leftx,261,leftx+257,287)
	Call recti(leftx,261,leftx+257,297)
C
C	draw the selection pins
C
	Call rgbcol(255,0,0)
	call rectfi(leftx+1+pins(1),288,leftx+1+pins(2),296)
C
C	draw the color bars
C
	Do I=1,256
		Call rgbcol(i-1,i-1,i-1)
		p1(1) = leftx+i
		p1(2) = 262
		p2(1) = leftx+i
		p2(2) = 274
		Call bgnlin
		Call v2i(p1)
		Call v2i(p2)
		Call endlin
		call rgbcol(luts(state,i,1),luts(state,i,2),luts(state,i,3))
		p1(1) = leftx+i
		p1(2) = 274
		p2(1) = leftx+i
		p2(2) = 286
		Call bgnlin
		Call v2i(p1)
		Call v2i(p2)
		Call endlin
	Enddo
C
C	and the lut lines
C
	Do k=1,4
		If (disp(state,k) .eq. 1) then
			if (k .eq. 1) then
				h = '000000FF'X
				Call cpack(h)
			Else if (k .eq. 2) then
				h = '0000FF00'X
				Call cpack(h)
			Else if (k .eq. 3) then
				h = '00FF0000'X
				Call cpack(h)
			Else if (k .eq. 4) then
				h = '00FFFFFF'X
				Call cpack(h)
			Endif
			Do i=1,255
				p1(1) = leftx+i
C				p1(2) = luts(state,i,k)+5
				p1(2) = (ed_pal(i,k)*255.0)+5
				p2(1) = leftx+i+1
C				p2(2) = luts(state,i+1,k)+5
				p2(2) = (ed_pal(i+1,k)*255.0)+5
				Call bgnlin
				Call v2i(p1)
				Call v2i(p2)
				Call endlin
			Enddo
		Endif
	Enddo
C
	Return
	End
C
C	routines to read/write pals to disk/shared memory
C
	subroutine pal_io(cmd,state,luts)

	Implicit None

	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../filer/filer_inc.inc'

	Integer*4	state,i,k,cmd
	Integer*4	luts(3,256,4)  !(image,voxel)(index)(r,g,b,a)
	Integer*4	pal(768)
	logical		error
C
C	check cmd type
C
	If (cmd .eq. IPC_READ) then
C
C	read pals
C
	    If (state .eq. 1) Then
		Call named_pal_io(FLR_PAL1_L_PAL,luts,"_ilut ",i)
		Call named_pal_io(FLR_PAL1_L_OPAC,luts,"_iopac ",i)
	    Else if (state .eq. 2) Then
		Call named_pal_io(FLR_PAL2_L_PAL,luts,"_vlut ",i)
		Call named_pal_io(FLR_PAL2_L_OPAC,luts,"_opacity ",i)
	    Else if (state .eq. 3) Then
		Call named_pal_io(FLR_PAL3_L_PAL,luts,"_texture ",i)
		Call named_pal_io(FLR_PAL3_L_OPAC,luts,"_texopac ",i)
	    Endif
C
C	done...
C
	Else if (cmd .eq. IPC_WRITE) then
C
C	write pals
C
	    If (state .eq. 1) Then
		Call named_pal_io(FLR_PAL1_S_PAL,luts,"_ilut ",i)
		Call named_pal_io(FLR_PAL1_S_OPAC,luts,"_iopac ",i)
	    Else if (state .eq. 2) Then
		Call named_pal_io(FLR_PAL2_S_PAL,luts,"_vlut ",i)
		Call named_pal_io(FLR_PAL2_S_OPAC,luts,"_opacity ",i)
	    Else if (state .eq. 3) Then
		Call named_pal_io(FLR_PAL3_S_PAL,luts,"_texture ",i)
		Call named_pal_io(FLR_PAL3_S_OPAC,luts,"_texopac ",i)
	    Endif
C
C	done
C
	Endif
C
	return
	end
C
C	subtroutine to smear out a user palette edit
C
	subroutine spread(xo,x,state,edit)

	Implicit None

	Include 'pals.inc'
	
	Integer*4	x1,x2,state,edit(3,4),luts(3,256,4)

	Integer*4	I,xo,x,k
	Real*4		f1,dy
C
C	swap X1 and X2 if needed (or exit if they are equal)
C
	if (x .gt. xo) then
		x1 = xo
		x2 = x
	Else if (xo .gt. x) then
		x1 = x
		x2 = xo
	Else
		return
	Endif
C
C	interpolate
C
	Do k=1,4
		If (edit(state,k) .eq. 1) then
			f1 = ed_pal(x1,k)
			dy = (ed_pal(x2,k) - F1)/float(x2-x1)
			Do i=x1,x2
				if (f1 .gt. 1.0) f1 = 1.0
				if (f1 .lt. 0.0) f1 = 0.0
				ed_pal(I,k) = f1
				f1=f1+dy
			Enddo
		Endif
	Enddo
C
	Return
	End
C
C	Routine to send a palette over IPC
C
	Subroutine  send_palette(luts,why)
C
	Implicit None
C
	Include '../libs/vt_inc.f'

	Integer*4	luts(3,256,4),i,temp(1024),why,plut(257)
C
	If (why .eq. IPC_V_VLUT) Then
		Do i=1,256
			temp(i) = luts(2,i,1)
			temp(i+256) = luts(2,i,2)
			temp(i+512) = luts(2,i,3)
			temp(i+768) = luts(2,i,4)
		Enddo
		Call pack_palette(temp,plut(2))
	Else if (why .eq. IPC_T_ILUT) Then
		Do i=1,256
			temp(i) = luts(1,i,1)
			temp(i+256) = luts(1,i,2)
			temp(i+512) = luts(1,i,3)
			temp(i+768) = luts(1,i,4)
		Enddo
		Call pack_palette(temp,plut(2))
	Else if (why .eq. IPC_V_ILUT) Then
		Do i=1,256
			temp(i) = luts(1,i,1)
			temp(i+256) = luts(1,i,2)
			temp(i+512) = luts(1,i,3)
			temp(i+768) = luts(1,i,4)
		Enddo
		Call pack_palette(temp,plut(2))
	Else if (why .eq. IPC_V_TLUT) Then
		Do i=1,256
			temp(i) = luts(3,i,1)
			temp(i+256) = luts(3,i,2)
			temp(i+512) = luts(3,i,3)
			temp(i+768) = luts(3,i,4)
		Enddo
		Call pack_palette(temp,plut(2))
	Endif
C
C	What to do...
C
	plut(1) = why
C
	Call send_ipc(257,plut)
C
	Return
	End
C
C

	Subroutine dirty_pals(state,v_ilut,t_ilut,v_vlut,v_tlut,
     +		t_tlut,wid)

	Implicit None

	Include 'fdevice.h'

	Integer*4	state,v_ilut,t_ilut,v_vlut,v_tlut,wid,t_tlut

	If (state .eq. 1) then
		v_ilut = 1
		t_ilut = 1
	Else if (state .eq. 2) then
		v_vlut = 1
	Else if (state .eq. 3) then
		v_tlut = 1
		t_tlut = 1
	Endif
	Call qenter(REDRAW,wid)

	Return
	End
C
C	Routine for named palette fileIO
C
	Subroutine named_pal_io(why,luts,name,state)

	Implicit None

	Include '../filer/filer_inc.inc'

	Integer*4	why,state,luts(3,256,4)
	Integer*4	pal(768),i,palette,oper,strlen
	External strlen
	Character*(*)	name
	Logical		error
C
C	Find out which palette to change (state)
C
C	Default = FLR_PAL1_L_PAL
C
	state = 1 ! pal1
	palette = 1 ! LUT
	oper = 1 ! read
	if (why .eq. FLR_PAL1_S_PAL) Then
		oper = 2
	Endif
	if (why .eq. FLR_PAL1_L_OPAC) Then
		palette = 2
	Endif
	if (why .eq. FLR_PAL1_S_OPAC) Then
		oper = 2
		palette = 2
	Endif
	if (why .eq. FLR_PAL2_S_OPAC) Then
		state = 2
		oper = 2
		palette = 2
	Endif
	if (why .eq. FLR_PAL2_L_OPAC) Then
		state = 2
		oper = 1
		palette = 2
	Endif
	if (why .eq. FLR_PAL2_S_PAL) Then
		state = 2
		oper = 2
		palette = 1
	Endif
	if (why .eq. FLR_PAL2_L_PAL) Then
		state = 2
		oper = 1
		palette = 1
	Endif
	if (why .eq. FLR_PAL3_S_OPAC) Then
		state = 3
		oper = 2
		palette = 2
	Endif
	if (why .eq. FLR_PAL3_L_OPAC) Then
		state = 3
		oper = 1
		palette = 2
	Endif
	if (why .eq. FLR_PAL3_S_PAL) Then
		state = 3
		oper = 2
		palette = 1
	Endif
	if (why .eq. FLR_PAL3_L_PAL) Then
		state = 3
		oper = 1
		palette = 1
	Endif
C
C	Read or write, palette or opacity
C
	If ((oper .eq. 2).and.(palette .eq.2)) Then
		Do i=1,256
			pal(i)=luts(state,i,4)
			pal(i+256)=i-1
			pal(i+512)=i-1
		Enddo
		Call write_pal(name(1:strlen(name)),pal,error)
		If (error .eq. .true.) Then
			write(*,10)"write to",name(1:strlen(name))
			Call bv_error("Unable to write to "//
     +				name(1:strlen(name))//"|"//
     +				"Check disk space and permissions}",0)
		Endif
	Else if ((oper .eq. 1).and.(palette .eq.2)) Then
		Call read_pal(name(1:strlen(name)),pal,error)
		If (error .eq. .true.) Then
			write(*,10)"read from",name(1:strlen(name))
		Else
			Do i=1,256
				luts(state,i,4) = pal(i)
			Enddo
		Endif
	Else if ((oper .eq. 2).and.(palette .eq.1)) Then
		Do i=1,256
			pal(i)=luts(state,i,1)
			pal(i+256)=luts(state,i,2)
			pal(i+512)=luts(state,i,3)
		Enddo
		Call write_pal(name(1:strlen(name)),pal,error)
		If (error .eq. .true.) Then
			write(*, 10)"write to",name(1:strlen(name))
			Call bv_error("Unable to write to "//
     +				name(1:strlen(name))//"|"//
     +				"Check disk space and permissions}",0)
		Endif
	Else if ((oper .eq. 1).and.(palette .eq.1)) Then
		Call read_pal(name(1:strlen(name)),pal,error)
		If (error .eq. .true.) Then
			write(*, 10)"read from",name(1:strlen(name))
		Else
			Do i=1,256
				luts(state,i,1) = pal(i)
				luts(state,i,2) = pal(i+256)
				luts(state,i,3) = pal(i+512)
			Enddo
		Endif
	Endif
10	Format("Error: Unable to ",A," the file:",A)
C
	Return
	End
C
C	Set the current histogram
C
	Subroutine sethist(hist,mhist,mihist,thist,tihist,
     +		use_i_hist,use_t_data)

	Implicit None

	Integer*4	hist(256),mhist(256),mihist(256),thist(256)
	Integer*4	tihist(256),use_i_hist,use_t_data

	Integer*4	i

	If ((use_i_hist .ne. 0).and.(use_t_data .ne. 0)) Then
		Do i=1,256
			hist(i) = tihist(i)
		Enddo
	Else if (use_t_data .ne. 0) Then
		Do i=1,256
			hist(i) = thist(i)
		Enddo
	Else if (use_i_hist .ne. 0) Then
		Do i=1,256
			hist(i) = mihist(i)
		Enddo
	Else
		Do i=1,256
			hist(i) = mhist(i)
		Enddo
	Endif

	Return
	End
C
C	Save the current histogram to disk
C
	Subroutine	save_hist(name,hist)

	Implicit None

	Character*(*)	name
	Integer*4	hist(256),i,j,strlen
	External strlen
	logical		found

	Inquire(exist=found,file=name)
	If (found) Then
		Open(66,file=name,form="formatted",status="old",err=999)
	Else
		Open(66,file=name,form="formatted",status="new",err=999)
	Endif

	j = 0
	Do i=1,256
		j = j + hist(i)
		write(66,10,err=998) i-1,char(9),hist(i),char(9),j
	Enddo
10	Format(I,A,I,A,I)

	Close(66,err=999)

	Return

998	Close(66,err=999)

999	Continue

	Call bv_error("Unable to write to "//
     +		name(1:strlen(name))//"|"//
     +		"Check disk space and permissions}",0)

	Return

	End
