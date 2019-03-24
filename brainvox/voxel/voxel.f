C 
C	To disable the MP_FIT options define the CPP var NO_MP_FIT
C	To disable the surface options define the CPP var NO_SURF
C	To disable the Howard disk options define the CPP var NO_DSPACE
C
	Program brainvox_voxel

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include '../filer/filer_inc.inc'
	Include 'voxeluif.inc'
	Include 'script.inc'

	Include 'measure.inc'
	Include 'rotsdef.inc'
	Include 'palettes.inc'
	Include 'windows.inc'
	Include 'voxel_globals.inc'
	Include 'prefs.f'

	Record	/rotsrec/	rots

	Logical		error,dirty,autosample
	Integer*4	MP_disp
	Real*4		frots(12),ftals(3),ftmp(8)
	Real*4		norm(4)
	Integer*4	status,i,state,mode,j,itmp(8)
	Integer*4	device
	Integer*4	x,y,xo,wleft,wbot,yo
	Integer*2	data
	Integer*4	rottable(3,8)
	Integer*4	thres,tempi,cindex,bcolor 
	Integer*4	running_script
C
	Integer*4	w2dpts(4096,4)   ! x,y,slice,color
	Integer*4	w3dpts(4096,4)   ! x,y,?,color
	Integer*4	temppts(4096,4),retvals(4)
	Integer*4	num2dpts,num3dpts
	Integer*4	winsave,label_over,h
C
	Integer*4	ix,iy,tx,ty
C	Integer*4	use_16bit,over_render,use_hardware,auto_render
C
	Character*250	tstr
	Character*250	tempstr
	Character*10 	clist(14)
C
C	PET data from _patient
C
	Character*200	inj_temp,pet_t_dir
	Integer*4	pet_min,pet_max,pet_avg,strlen
C
	Integer*4	grid_menu,back_menu,savep_menu,loadp_menu
C
	Record /ditem/	dialog(100)
C
C	FOR FAT LINES
C
	Integer*4	FAT,tal_over,clippingon,clipshade
C
C	IPC defines
C
	Integer*4	size,message(IPC_SHM_SIZE),templut(1024)
C
	Integer*4	iargc,slicetoz,ztoslice,bv_mkdir
	Integer*4		ZtoSlice_t
	Real*4		ftemp

	Integer*4 cmd_hist_len,cmd_hist_pos
	Integer*4 cmd_hist_max
	Parameter (cmd_hist_max = 20)
	Character*200 cmd_hist(cmd_hist_max)

	Real*4 bv_frand
	external 	iargc,slicetoz,ztoslice,bv_mkdir,ZtoSlice_t,
     +		bv_frand,strlen
C
	common	/fat_com/fat,clippingon,tal_over,clipshade
C
	data		rottable/90 ,0  ,0  ,270,0  ,0  ,
     +				 0  ,90 ,0  ,0  ,270,0  ,
     +				 0  ,0  ,0  ,0  ,180,0  ,
     +				 20 ,40 ,0  ,20 ,320,0  /
	data	clist/'Co:Black  ',
     +		      'Co:Red    ',
     +		      'Co:Green  ',
     +		      'Co:Yellow ',
     +		      'Co:Blue   ',
     +		      'Co:Magenta',
     +		      'Co:Cyan   ',
     +		      'Co:White  ',
     +		      'Co:Coral  ',
     +		      'Co:Chartre',
     +		      'Co:SpGreen',
     +		      'Co:SlateBl',
     +		      'Co:BlueVio',
     +		      'Co:OrangeR'/
C
C	zoom/trans for output images
C
C	data		offset2d/0,0/
	data		offset3d/0,0/
C	data		zoom2d/1/
	data		zoom3d/1/
C
	cmd_hist_len = 0
	cmd_hist_pos = 0
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
	if (iargc() .lt. 1) then
2		write(*, 17)
17	Format("This program must be launched from within brainvox.") 
		Call bv_exit(1)
	Endif
	call getarg_nq(iargc(),tstr)
	read(unit=tstr,fmt=*,err=2) shm_data
C
        Call getarg_nq(1,tstr)   
	If (tstr .eq. 'DIALOG_PSET') then
		Call rename_pointsets(shm_data)
		Call bv_exit(0)
	Endif
	If (tstr .eq. 'DIALOG_MPIO') then
		Call mp_fit_settings_io(shm_data)
		Call bv_exit(0)
	Endif
	If (tstr .eq. 'DIALOG_LMOD') then
		Call lighting_model_control(shm_data)
		Call bv_exit(0)
	Endif
	If (tstr .eq. 'DIALOG_TEXTURE') then
		call getarg_nq(2,tstr)
		Call texture_control(shm_data,tstr)
		Call bv_exit(0)
	Endif
	If (tstr .eq. 'DIALOG_MOVIE') then
		Call movieola(shm_data)
		Call bv_exit(0)
	Endif
C
C	print version and program ID
C
	If ((iargc() .lt. 1).or.(bv_test_option(OPTS_TESTING))) Then
	write(*, 1)version,com_date
1	Format(" Welcome to BRAINVOX_VOXEL version:",A15,/,
     +	       " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
	Endif
C
	running_script = 0
        numrgns=0  
	win_save = -1
C
	image3dsize = 445
	FAT = 0
	use_16bit = 0
	over_render = 0
	use_hardware = 0
        use_hwinterp = 0
	auto_render = 0
	clippingon = 0
	clipshade = 0
	autosample = .true.
	bcolor = 0
	tal_over = 0
	label_over = 0
	rots.sthickness = 6.64
	rots.ipixel = 1.0
	rots.n_slices(1) = 15  ! count
	rots.n_slices(2) = 0  ! view
	rots.x_off = 0.0	! xoffset
	rots.xshift= 0.0
	rots.outname = "sampled_%%%.pic "
	Do i= 1,2
		rots.slew(i) = 0
		rots.tilt(i) = 0
	Enddo
	Do i= 1,3
		rots.uptrans(i) = 0.0
		rots.uprots(i) = 0.0
		rots.fpnorm(i) = 0.0
		Do j=1,4
			rots.fppoints(j,i) = 0.0
		Enddo
		rots.upxaxis(i) = 0.0
		rots.upyaxis(i) = 0.0
		rots.upnorm(i) = 0.0
	Enddo
	rots.upxaxis(1) = 1.0
	rots.upyaxis(2) = 1.0
	rots.upnorm(3) = 1.0
	rots.fpnorm(3) = 1.0
	MP_disp = 9
	pointtype = 2  ! tal points
	data_layer = 0 ! first pane of measurement data
C
	Call init_measure
C
C	init the calibration spaces
C
	Call init_tal
	Call init_dspace
C
C	I am NOT the shared memory owner!!! (there can be only one!!)
C
	shmem_owner = 0
	Call vt_init_shmem
C
C       read the rgnsdef file
C
        Call read_rgns(tstr,error)
C
C	A pref...
C
	Call get_pref_i(PREF_3DHWINTERP,use_hwinterp)
C
C	get the window
C
	x = (getgde(GDXPMA)-485)
	y = (getgde(GDYPMA)-385)
	call prefpo(x,x+485,y,y+385)
	wid=winope("voxel",5)
	Call doubleb
	Call RGBmod
	Call gconfi
C
C	define dashed linestyle
C
	h = '00000303'X
	call deflin(2768,h)
C
C	add devices
C
	Call qdevic(WINQUI)
	Call qdevic(WINFRE)
	Call qdevic(WINTHA)
	Call qdevic(UPARRO)
	Call qdevic(DOWNAR)
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('voxeluif.r',1,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
C	Read PET patient data
C
        Call read_patient_pet("_patient ",inj_temp,pet_min,
     +                          pet_max,pet_avg,error)
C
C	Build the menus
C
	Call build_menus(dialog,savep_menu,loadp_menu,grid_menu,
     +                  im_menu,back_menu,error)
C
C	init the data set (both 8 bit volumes)
C
	Call	vl_init_hw(%val(0),%val(1),myset)
	i = realimages+((realimages - 1) * interpimages)
	Call	vl_data(%val(imagesize),%val(imagesize),%val(i),
     +			%val(images_addr),myset)
	Call	vl_aux_data(%val(1),%val(texture_addr),myset)
	Call	recalc_volume_limits(myset)
C
C	set the squeeze factor
C
	If (realimages .gt. 2) then
                squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)

		Call vl_setsqu(squeezefactor,myset)
		If (use_hwinterp .eq. 0) Then
			Call vl_setinterp(%val(0),myset)
		Else
			Call vl_setinterp(%val(interpimages),myset)
		Endif
	Endif
	If (bv_test_option(OPTS_RJF)) Then
		write(*, *)"Brainvox - Squeeze factor =",squeezefactor
		write(*, *)"Brainvox - Real Images =",realimages
		write(*, *)"Brainvox - Interp Images =",interpimages
	Endif
C
C	init the slider positions
C
	Do I=1,6
		rots.rots(i)= 0
	Enddo
	rots.rots(7) = 256
C
C	init luts
C
	Call read_pal("_vlut ",palette,error)
	Call read_pal("_opacity ",opacity,error)
	Call read_pal("_texture ",texture_c,error)
	Call read_pal("_texopac ",texture_o,error)
	Call setpal(palette,opacity,texture_c,texture_o,bcolor,myset)
	Call read_pal("_iopac ",templut,error)
	Call read_pal("_ilut ",pal_2d,error)
	Do i=769,1024
		pal_2d(i) = templut(i)
	Enddo
C
C	no Grids...
C
	grid1 = 0.0
C	grid2 = 0.0
C
C	open the voxel image window
C
	x=(0)
	y=(getgde(GDYPMA)-image3dsize)
	Call minsiz(image3dsize,image3dsize)
	call prefpo2(x,y+image3dsize-1)
C	call prefpo(x,x+image3dsize-1,y,y+image3dsize-1)
	wid2 = winope("3DImage",7)
	Call minsiz(image3dsize,image3dsize)
	Call wincon
	Call doubleb
	Call RGBmod
	If (getgde(GDBIST) .ne. 0) Then
		Call stensi(2)
		If (bv_test_option(OPTS_RJF)) Then
			write(*, *)"2 Stencil planes enabled"
		Endif
	Endif
	Call gconfi
C
C	open the 2D image window
C
	Call init_window(i)
C
C	initial color index
C
	cindex = 2
	dialog(13).text = clist(cindex+1)
	dialog(13).tlength = 10 

	linedata.color = 2
	dialog(measurecolor).text = clist(linedata.color+1)
	dialog(measurecolor).tlength = 10

C
	Call setup_dialog(rots,dialog,wid,myset,0)
C
C	allocate memory for backbuffer
C	4 bytes image 2 bytes zbuffer
C
	Call fmalloc(save_ptr,image3dsize*image3dsize*6)  
	if (save_ptr .eq. 0) goto 999
	call memclear(%val(save_ptr),image3dsize*image3dsize*6)
	zbufptr = save_ptr + (image3dsize*image3dsize*4)
	Call vl_image(%val(image3dsize),%val(image3dsize),%val(save_ptr),
     +			%val(zbufptr),myset)
C
C	Queue devices
C
	Call qdevic(INPTCH)
	Call qdevic(MOUSE3)
	Call qdevic(MOUSE2)
	Call qdevic(F1KEY) 
	Call qdevic(PADAST) 
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
C
	dirty = .true.
	wind2d(curwin).dirty = .true.
	num2dpts = 0
	num3dpts = 0
	winput = wid
	rendermode = 0
C
	Call update_texture_functions(1)
C
C	redraw comes from setup_dialog
C
C	TOP OF THE EVENT LOOP
C	if scripting, play one cmd and if no pending event, more cmds
C
10	if (running_script .ne. 0) then
		if (qtest() .ne. 0) goto 20  ! flush the event queue first!
		j = 0;
		call continue_command(myset,running_script,i,ftmp,tstr)
		if (i .eq. SCR_CMD_ROT) then
			rots.rots(1) = ftmp(1)
			rots.rots(2) = ftmp(2)
			rots.rots(3) = ftmp(3)
			Call setup_dialog(rots,dialog,wid,myset,0)
			Call reproject2d3d(num2dpts,w2dpts,num3dpts,w3dpts)
			j = 1;
		else if (i.eq.SCR_CMD_PAL) then
			j = FLR_PAL2_L_PAL
			if (ftmp(1) .eq. 1) then
				j = FLR_PAL3_L_PAL
			else if (ftmp(1) .eq. 2) then
				j = FLR_PAL2_L_OPAC
			else if (ftmp(1) .eq. 3) then
				j = FLR_PAL3_L_OPAC
			else if (ftmp(1) .eq. 4) then
				j = FLR_PAL1_L_PAL
			else if (ftmp(1) .eq. 5) then
				j = FLR_PAL1_L_OPAC
			endif
			Call send_string(tstr,j,index(tstr,char(0)))
			if (ftmp(1) .lt. 4) then
				Call vl_getpals2(palette,opacity,texture_c,
     +					texture_o,bcolor,myset)
				Call setpal(palette,opacity,texture_c,
     +					texture_o,bcolor,myset)
			endif
			j = 1;
		else if (i.eq.SCR_CMD_COLOR) then
			Call vl_getpals2(palette,opacity,texture_c,texture_o,
     +					bcolor,myset)
			Call setpal(palette,opacity,texture_c,texture_o,
     +					bcolor,myset)
			j = 1;
		else if (i .eq. SCR_CMD_FILTER) then
			Call qenter(REDRAW,wid2)
		else if (i .eq. SCR_CMD_HWRENDER) then
			use_hardware = ftmp(1)
			j = 1
		else if (i .eq. SCR_CMD_AUTORENDER) then
			auto_render = ftmp(1)
			j = 1
		else if (i .eq. SCR_CMD_TEXRENDER) then
			use_16bit = ftmp(1)
			j = 1
		else if (i .eq. SCR_CMD_RENDER) then
			dirty = .true.
			Call qenter(REDRAW,wid2)
		else if (i .eq. SCR_CMD_SAVE) then
		        tempi = winget()
			Call winset(wid2)
			Call save_rgb(tstr(1:index(tstr,char(0))-1)//" ",0)
			Call winset(tempi)
		else if (i .eq. SCR_CMD_REBOUND) then
			message(1) = IPC_TEXUTIL
			message(2) = IPC_TEX_UTIL_REBOUND
			message(3) = 0
			message(4) = 255
			Call send_ipc(4,message)
			Call qenter(REDRAW,0)
			j = 1
		else if (i .eq. SCR_CMD_DATA) then
C
C	which, interp, bits|flag,inmin,inmax,outmin,outmax
C	flag : 256 = comp min/max (low order bits = bits)
C
			Do j=1,7
				itmp(j) = ftmp(j)
			Enddo
			volumedepth = iand(itmp(3),255)
			volume_bswap = 0
			if (itmp(3) .ge. 512) then
				volume_bswap = 1
			endif
			itmp(3) = 8
			volume_lims(1) = itmp(4)
			volume_lims(2) = itmp(5)
			Call volume_loadimages(tstr(1:index(tstr,char(0))-1),
     +				itmp(2),error,wid,itmp(1),itmp(3))
			Call vl_dirty(%val(2),myset)
			Call send_ipc(1,IPC_TEXVOLUME)
			Call qenter(REDRAW,wid)
			j = 1;
		else if (i .eq. SCR_CMD_LIGHTING) then
			Call set_intrinsic_lighting(ftmp,rendermode)
			j = 1;
		endif
		if ((j .ne. 0) .and. (auto_render .eq. 1)) then
			dirty = .true.
			Call qenter(REDRAW,wid2)
		endif
	endif
20	device=qread(data)
C
C	check for change in input focus
C
	if (device .eq. INPTCH) then
		i = data
		if (data .ne. 0) call winset(i)
		winput = data
		goto 10
	Endif
C
C	FAT lines stuff
C
	If ((device .eq. F1KEY).and.(data .eq. 1)) then
		FAT = FAT + 1
		If (FAT .gt. 8) FAT = 0
		Call Qenter(REDRAW,0)
		goto 10
	Endif
C
C	pointset thinning
C
	If ((device .eq. PADAST).and.(data .eq. 1)) then
		j = 0
		Do i=1,num3dpts,2
			j = j + 1
			w3dpts(j,1) = w3dpts(i,1)
			w3dpts(j,2) = w3dpts(i,2)
			w3dpts(j,3) = w3dpts(i,3)
			w3dpts(j,4) = w3dpts(i,4)
		Enddo
		num3dpts = j
		j = 0
		Do i=1,num2dpts,2
			j = j + 1
			w2dpts(j,1) = w2dpts(i,1)
			w2dpts(j,2) = w2dpts(i,2)
			w2dpts(j,3) = w2dpts(i,3)
			w2dpts(j,4) = w2dpts(i,4)
		Enddo
		num2dpts = j
		Call do_measure(w2dpts,num2dpts,w3dpts,
     +				num3dpts,myset,dialog,1)	
		Call Qenter(REDRAW,0)
		goto 10
	Endif
C
C	Handle text history buffer
C
	If ((device .eq. UPARRO).and.(data .eq. 1).and.
     +      (dialog(1).tpos .eq. CMD_EDIT)) Then
                cmd_hist_pos = cmd_hist_pos + 1
                if (cmd_hist_pos .gt. cmd_hist_len) Then
                        cmd_hist_pos = cmd_hist_len
                endif
                dialog(CMD_EDIT).text = cmd_hist(cmd_hist_pos)
                i = index(cmd_hist(cmd_hist_pos),char(0))
                dialog(CMD_EDIT).tlength = i-1
                dialog(CMD_EDIT).tpos = i-1
		Call qenter(REDRAW,wid)
		goto 10
	Endif
	If ((device .eq. DOWNAR).and.(data .eq. 1).and.
     +      (dialog(1).tpos .eq. CMD_EDIT)) Then
                cmd_hist_pos = cmd_hist_pos - 1
                if (cmd_hist_pos .lt. 0) Then
                        cmd_hist_pos = 0
                	dialog(CMD_EDIT).tlength = 0
                	dialog(CMD_EDIT).tpos = 0
                else
                	dialog(CMD_EDIT).text = cmd_hist(cmd_hist_pos)
               		i = index(cmd_hist(cmd_hist_pos),char(0))
                	dialog(CMD_EDIT).tlength = i-1
                	dialog(CMD_EDIT).tpos = i-1
                endif
		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C	timer stuff
C
	if (device .eq. TIMER0) then
		size = IPC_SHM_SIZE
		call check_ipc(size,message)
		if (message(1) .eq. IPC_QUIT) goto 999
		if (message(1) .eq. IPC_F_RETURN) then
			if (message(2) .eq. FLR_CMD_FILE) then
				Call get_filename(tstr,message(3),i)
				if (i .ne. 0) then
	if (running_script .ne. 0) then
		write(*,*) "Warning: script already running, cmd ignored"
	else
		call execute_command("chain,'"//tstr(1:i)//"'")
		running_script = 1
	endif
				endif
			endif
		endif
		if (message(1) .eq. IPC_VLUTS) then
			Call send_ipc(1,IPC_OK)
			Call read_pal("_vlut ",palette,error)
			Call read_pal("_opacity ",opacity,error)
			Call setpal(palette,opacity,texture_c,texture_o,
     +				bcolor,myset)
			Call read_pal("_ilut ",pal_2d,error)
			Call Qenter(REDRAW,0)
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Endif
		if (message(1) .eq. IPC_TEXFUNCS) then
C
C	New texturing functions
C
			Call send_ipc(1,IPC_OK)
			Call update_texture_functions(1)
		Endif
		if ((message(1) .eq. IPC_TEXCLIP).or.
     +			(message(1) .eq. IPC_TEXFILL) .or.
     +			(message(1) .eq. IPC_TEXUTIL)) then
C
C	Texture volume clipping functions
C
			Call clip_texture(w2dpts,num2dpts,myset,
     +			    message(1),message(2),message(3),message(4))
C
C	The texture volume is new!!
C
			Call update_texture_functions(0)
			Call send_ipc(1,IPC_OK)
			message(1) = IPC_I_HISTO
			message(258) = IPC_H_TEXTUREVOL
			Call send_ipc(258,message)
			Call vl_dirty(%val(2),myset)
			If ((auto_render .eq. 1) .and. 
     +				(use_16bit .eq. 1)) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Endif
		if (message(1) .eq. IPC_TEXVOLUME) then
C
C	A new texture volume (send histogram to pals, resample some windows)
C
			Call update_texture_functions(0)
			Call send_ipc(1,IPC_OK)
			message(1) = IPC_I_HISTO
			message(258) = IPC_H_TEXTUREVOL
			Call send_ipc(258,message)
			Call vl_dirty(%val(2),myset)
			If ((auto_render .eq. 1) .and. 
     +				(use_16bit .eq. 1)) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
			goto 10
		Endif
		if (message(1) .eq. IPC_PNAMES) then
			Call send_ipc(1,IPC_OK)
        		Call update_pset_menu(loadp_menu,3000,'./ ',0)
        		Call update_pset_menu(savep_menu,4000,'./ ',1)
		Endif
		if (message(1) .eq. IPC_MPIOLOAD) then
			Call MP_FIT_load_settings(message(2),rots)
			Call send_ipc(1,IPC_OK)
			tstr = rots.outname
			i = strlen(tstr)
			if (i .lt. 0) i = 1
			dialog(tempedit).text = tstr(1:i)
			dialog(tempedit).tlength = i
			write(dialog(thickedit).text,350) rots.sthickness
			dialog(thickedit).tlength = 5
			write(dialog(ipixeledit).text,352) rots.ipixel
			dialog(ipixeledit).tlength = 7
			write(dialog(countedit).text,349) rots.n_slices(1)
			dialog(countedit).tlength = 3
			write(dialog(offedit).text,351) rots.x_off
			dialog(offedit).tlength = 5
			write(dialog(cslewedit).text,351) rots.slew(1)
			dialog(cslewedit).tlength = 5
			write(dialog(ctiltedit).text,351) rots.tilt(1)
			dialog(ctiltedit).tlength = 5
			write(dialog(xshiftedit).text,351) rots.xshift
			dialog(xshiftedit).tlength = 5
			Call calc_up(rots,interpixel)
			Call dirty_window(-1,2)
			Call setup_dialog(rots,dialog,wid,myset,0)
		Endif
		if (message(1) .eq. IPC_MPIOSAVE) then
			Call MP_FIT_save_settings(message(2),rots)
			Call send_ipc(1,IPC_OK)
		Endif
C
C	Messages from the palette editor and lighting model dialog
C
		if (message(1) .eq. IPC_V_VLUT) then
			Call unpack_palette(message(2),templut)
			Do i=1,256
				palette(i) = templut(i)
				palette(i+256) = templut(i+256)
				palette(i+512) = templut(i+512)
				opacity(i) = templut(i+768)
			Enddo
			Call setpal(palette,opacity,texture_c,texture_o,
     +					bcolor,myset)
			Call send_ipc(1,IPC_OK)
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Else if (message(1) .eq. IPC_V_ILUT) then
			Call unpack_palette(message(2),templut)
			Do i=1,1024
				pal_2d(i) = templut(i)
			Enddo
			Call send_ipc(1,IPC_OK)
			Call update_texture_functions(1)
			Call qenter(REDRAW,0)
		Else if (message(1) .eq. IPC_V_TLUT) then
			Call unpack_palette(message(2),templut)
			Do i=1,256
				texture_c(i) = templut(i)
				texture_c(i+256) = templut(i+256)
				texture_c(i+512) = templut(i+512)
				texture_o(i) = templut(i+768)
			Enddo
			Call setpal(palette,opacity,texture_c,texture_o,
     +					bcolor,myset)
			Call send_ipc(1,IPC_OK)
			Call update_texture_functions(1)
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Else if (message(1) .eq. IPC_L_INTRINSIC) then
			Call set_intrinsic_lighting(message(2),rendermode)
			Call send_ipc(1,IPC_OK)
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Else if (message(1) .eq. IPC_L_POSTLIGHT) then
			Call wait_cursor
			Call do_postlight(%val(save_ptr),
     +				%val(zbufptr),
     +				%val(save_ptr),
     +				%val(image3dsize),
     +				%val(image3dsize),
     +				message(2))
			Call arrow_cursor
			Call Qenter(REDRAW,wid2)
			Call send_ipc(1,IPC_OK)
		Else if (message(1) .eq. IPC_L_FILTER) then
			Call wait_cursor
			Call do_filter_proc(%val(save_ptr),
     +				%val(zbufptr),%val(image3dsize),
     +				%val(image3dsize),%val(message(2)))
			If (message(2) .le. 11) Then
				Call Qenter(REDRAW,wid2)
			Else 
				Call do_measure(w2dpts,num2dpts,w3dpts,
     +					num3dpts,myset,dialog,1)	
			Endif
			Call arrow_cursor
			Call send_ipc(1,IPC_OK)
		Else if (message(1) .eq. IPC_T_ARBREQUEST) then
			If (wind2d(curwin).mode .eq. WIN_ARB_MODE) Then
				message(1) = IPC_T_ARBREPLY
				message(2) = 1
				message(3) = wind2d(curwin).rots(1)
				message(4) = wind2d(curwin).rots(2)
				message(5) = wind2d(curwin).rots(3)
				message(6) = wind2d(curwin).rots(4)
				Call send_ipc(6,message)
			Else
				message(1) = IPC_T_ARBREPLY
				message(2) = 0
				Call send_ipc(6,message)
			Endif
		Endif
		goto 10
	Endif
C
C	Handle mousedowns
C
	Call handle_mousedown(device,data,rots,dialog,
     +		MP_disp,cindex,w2dpts,w3dpts,num2dpts,num3dpts,frots,
     +		status,dirty)
C
C	Return codes
C
	If (status .eq. 10) goto 10
	If (status .eq. 7001) goto 7001
C
C	handle redraw
C
	If (device .eq. REDRAW) then	! redraw screen
C
		tempi = winget()
		If ((data .eq. 0).or.(data .eq. wid2)) Then
C
C	first the 3D image window
C
		call winset(wid2)
		Call RESHAP	! do not forget !!!
		call getsiz(ix,iy)
		Call ortho2(0.0,float(ix-1),0.0,float(iy-1))
C
		if (dirty) then
			Call wait_cursor
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
			Call vl_m_render(%val(1),%val(status),
     +				%val(MOUSE3),
     +				i,myset,%val(0))
C
C	recompute the measurements
C
			Call do_measure(w2dpts,num2dpts,w3dpts,
     +  			num3dpts,myset,dialog,1)
C
			Call arrow_cursor
C
C	if the user aborted then clear the queue of mouse down events
C
			If (i .eq. 4) Then
				Call qreset
				Call qenter(REDRAW,0)
			Endif
C
C	reset the dirty flag and the points
C
			dirty = .false.
		Endif
C
		Call ortho(0.0,float(ix-1),0.0,float(iy-1),
     +			-float(image3dsize)/2.0,
     +			float(image3dsize)/2.0)
C
		Call bumpoffset(offset3d,zoom3d,
     +			0,0,image3dsize,image3dsize,ix,iy)               
		Call draw_image_i4(image3dsize,image3dsize,ix,iy,
     +			offset3d,zoom3d,%val(save_ptr),%val(zbufptr))
C		
     		Call paintpoints(num3dpts,w3dpts,-1,
     +				zoom3d,offset3d,myset)    
		if (grid1 .gt. 0.0) call draw_grid(zoom3d,gcol1,grid1)
C
C	Draw 3D cursor overlay
C
		If (clippingon .ne. 0) then
			call zbuffe(.true.)
			Call zfunct(ZFLESS)
		Else
			call zbuffe(.false.)
		Endif
		Call draw_3d_cursor(offset3d,zoom3d)
C
C	Draw Tal_overlays (if desired)
C
		If (clippingon .ne. 0) then
			call zbuffe(.true.)
			Call zfunct(ZFLESS)
		Else
			call zbuffe(.false.)
		Endif
	        Call draw_tal_overlay(myset,imagesize,
     +          	tal_over,offset3d,zoom3d,gcol1)
C
		Call ortho(0.0,float(ix-1),0.0,float(iy-1),
     +			-float(image3dsize)/2.0,
     +			float(image3dsize)/2.0)
C
C	Draw the MP_FIT graphics if requested
C
		If (wind2d(curwin).mode .eq. WIN_MPFIT_MODE) then
        		Call draw_mp_fit_voxel(rots,myset,imagesize,
     +				interpixel,MP_disp,offset3d,zoom3d)
		Endif
C
C	Draw Dspace graphics if requested
C
		Call draw_dspace_voxel(rots,myset,imagesize,interpixel,
     +          	offset3d,zoom3d)
C
C	Clean up
C
		Call ortho2(0.0,float(ix-1),0.0,float(iy-1))
		Call swapbu
		Call zbuffe(.false.)
	Endif
C
C	Check for all the 2D windows
C
	winsave = curwin
	Do curwin=1,MAX_WINDOWS
		If ((wind2d(curwin).wid .ne.-1).and.((data.eq.0)
     +			.or.(data .eq. wind2d(curwin).wid))) Then
C
C	next the 2D image window
C
		call winset(wind2d(curwin).wid)
		Call RESHAP	! do not forget !!!
C		Call set_cindex_table
		call getsiz(ix,iy)
		Call ortho2(0.0,float(ix),0.0,float(iy))
C
C	Do a temp swap
C
		Call sel_window(curwin,0,myset,rots)
C
C	Resample images if needed
C
		if (wind2d(curwin).dirty) then
			If (wind2d(curwin).mode .eq. WIN_ARB_MODE) then
C
C	Arbitrary
C
				Do status = 1,3
					frots(status) = 
     +					wind2d(curwin).rots(status)
				Enddo
				frots(4) = wind2d(curwin).rots(4)-256
				Call wait_cursor
				Call vl_resample(%val(imagesize),
     +					%val(imagesize),
     +					%val(wind2d(curwin).im2d_ptr),
     +					frots(1),frots(4),myset)
				If (wind2d(curwin).texture_mode .ne. 0)Then
					Call vl_resample16(%val(imagesize),
     +						%val(imagesize),
     +					    %val(wind2d(curwin).tex2d_ptr),
     +						frots(1),frots(4),myset)
				Endif
				Call calc_image_histo(curwin)
				If (curwin.eq.winsave) Call send_image_histo
				Call arrow_cursor
			Else if (wind2d(curwin).mode .eq. 
     +				WIN_MPFIT_MODE) then
C
C	MP_FIT
C       Get the center point
C
				frots(4) = rots.sthickness/interpixel
                		Do status=1,3
                       		frots(status)=rots.upcenter(status) +
     +                          	float(-wind2d(curwin).n_slice)*
     +					frots(4)*rots.upnorm(status)
                		Enddo
C
				Call wait_cursor
				call vl_resample_axis(%val(imagesize),
     +					%val(imagesize),
     +					%val(wind2d(curwin).im2d_ptr),
     +					rots.upxaxis(1),rots.upyaxis(1),
     +					rots.upnorm(1),frots(1),
     +					myset,1.0)  !ratio = MRI/MRI
				If (wind2d(curwin).texture_mode .ne. 0)Then
				call vl_resample_axis16(%val(imagesize),
     +					%val(imagesize),
     +					%val(wind2d(curwin).tex2d_ptr),
     +					rots.upxaxis(1),rots.upyaxis(1),
     +					rots.upnorm(1),frots(1),
     +					myset,1.0)  !ratio = MRI/MRI
				Endif
				Call calc_image_histo(curwin)
				If (curwin.eq.winsave) Call send_image_histo
				Call arrow_cursor
			Else
				Call get_2dimage(wind2d(curwin).inum2d,
     +					%val(wind2d(curwin).im2d_ptr),1)
				If (wind2d(curwin).texture_mode .ne. 0)Then
				Call get_2dimage(wind2d(curwin).inum2d,
     +					%val(wind2d(curwin).tex2d_ptr),2)
				Endif
				Call calc_image_histo(curwin)
				If (curwin.eq.winsave) Call send_image_histo
			Endif
			wind2d(curwin).dirty = .false.
		Endif
		Call bumpoffset(wind2d(curwin).offset2d,
     +			wind2d(curwin).zoom2d,
     +			0,0,imagesize,imagesize,ix,iy)               
		Call draw_image_i2(imagesize,imagesize,ix,iy,
     +			wind2d(curwin).offset2d,
     +			wind2d(curwin).zoom2d,
     +			%val(wind2d(curwin).im2d_ptr),
     +			%val(wind2d(curwin).tex2d_ptr),
     +			wind2d(curwin).texture_mode)
		Call ortho2(0.0,float(ix),0.0,float(iy))
		Call paintpoints(num2dpts,w2dpts,wind2d(curwin).inum2d,
     +			wind2d(curwin).zoom2d,wind2d(curwin).offset2d,myset)
C
		Call draw_3d_cursor_arb(wind2d(curwin).offset2d,
     +			wind2d(curwin).zoom2d,wind2d(curwin).mode)
C
		If (wind2d(curwin).mode .eq. WIN_ARB_MODE) then
	        	Call draw_tal_arb_overlay(tal_over,
     +				wind2d(curwin).offset2d,
     +				wind2d(curwin).zoom2d,
     +				wind2d(curwin).gcol)
			Call bv_set_color(wind2d(curwin).gcol)
			If (label_over .ne. 0) Then
			Call vl_draw_labels_cutplane(wind2d(curwin).offset2d,
     +				%val(wind2d(curwin).zoom2d),%val(15))
			Endif
		Endif
		If (wind2d(curwin).grid .gt. 0.0) Then
			call draw_grid(wind2d(curwin).zoom2d,
     +				wind2d(curwin).gcol,
     +				wind2d(curwin).grid)
		Endif
		Call ortho2(0.0,float(ix),0.0,float(iy))
		Call swapbu
		Endif
	Enddo
C
C	Do a temp swap
C
		Call sel_window(winsave,0,myset,rots)
C
		If ((data .eq. 0).or.(data .eq. wid)) Then
C
C	next the dialog window
C
		call winset(wid)
		Call RESHAP	! do not forget !!!
		Call ortho2(0.0,485.0,0.0,385.0)
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) then
			 goto 999     
		Endif
		Call draw_cube(myset,rots,MP_disp)
		Call ortho2(0.0,485.0,0.0,385.0)
		Call swapbu
		Endif
		if (tempi .ne. 0) call winset(tempi)
C
C	image saves
C
		If (win_save .eq. wid2) then
			call save_rgb("image_3d_%%%.rgb ",1)
			win_save = -1
		Else if (win_save .ne. -1) then
			call save_rgb("image_2d_%%%.rgb ",1)
			win_save = -1
		Endif
C
		goto 10
	Endif
C
C	look for right mouse (MOUSE1) down in image window
C
	Call	handle_mouse1down(device,data,rots,dialog,
     +          cindex,status)
C
C	Handle jump cases
C
	If (status .eq. 10) goto 10
C
C	if the input window is NOT the dialog window do nothing...
C
	if (winput .ne. wid) goto 10
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) goto 999    
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		mode = 0
		If (data .eq. 11) then   !render
			dirty = .true.
			Call qenter(REDRAW,wid2)
		Else if (data .eq. CMD_PLAY) then
519     Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A," &")
			write(tstr,519) shm_data,FLR_CMD_FILE,
     +                          '"Select command file to play"',
     +                          '"*.bvc"'
			Call bv_system_path(tstr)
		Else if (data .eq. samplebutton) then
			wind2d(curwin).dirty = .true.
			Call qenter(REDRAW,wind2d(curwin).wid)
		Else if (data.eq.pntadd) then
			ftals(1) = 0.0
			i = dialog(talxedit).tlength
			read(dialog(talxedit).text(1:i),*,
     +				err=520,end=520) ftals(1)
520			ftals(2) = 0.0
			i = dialog(talyedit).tlength
			read(dialog(talyedit).text(1:i),*,
     +				err=521,end=521) ftals(2)
521			ftals(3) = 0.0
			i = dialog(talzedit).tlength
			read(dialog(talzedit).text(1:i),*,
     +				err=522,end=522) ftals(3)
522			num2dpts = num2dpts + 1
			if (num2dpts .gt. 4095)	num2dpts = 4095
			if (pointtype .eq. 2) then
				Call tal_tal2point(ftals,
     +					w2dpts(num2dpts,1),
     +					w2dpts(num2dpts,2),
     +					w2dpts(num2dpts,3))
			Else if (pointtype .eq. 3) then
				Call disk2point(ftals,
     +					w2dpts(num2dpts,1),
     +					w2dpts(num2dpts,2),
     +					w2dpts(num2dpts,3))
			Else
				w2dpts(num2dpts,1)=ftals(1)/interpixel
				w2dpts(num2dpts,2)=ftals(2)/interpixel
				w2dpts(num2dpts,3)=ftals(3)/
     +					(interpixel*squeezefactor)
			Endif
			w2dpts(num2dpts,4) = cindex
			Call do_measure(w2dpts,num2dpts,w3dpts,
     +				num3dpts,myset,dialog,1)	
			Call qenter(REDRAW,0)
			goto 7001
		Endif
		goto 10
	Endif
C
C	Handle non-editable text events
C
	If (device .eq.  DLG_TXTEVNT) Then
		If ((data .ge. measuretext).and.
     +			(data .le. measuretext+4)) Then

			Call next_meas_layer
			Call do_measure(w2dpts,num2dpts,w3dpts,
     +				num3dpts,myset,dialog,1)	
			Call qenter(REDRAW,wid)

		Endif
		goto 10
	Endif
C
C	Handle text edit events
C
	If (device .eq.  DLG_EDTEVNT) then
		ftemp = 0
		i = dialog(data).tlength
		if (data .eq. tempedit) then
			rots.outname = dialog(data).text(1:i)//' '
			ftemp = 0
		Else if (data .eq. CMD_EDIT) Then
C
C  Clear the text, but keep focus...
C
			if (i .ne. 0) then
	if (running_script .ne. 0) then
		write(*,*) "Warning: script already running, cmd ignored"
	else
C
C  Record command
C
		Do i=cmd_hist_len+1,2,-1
			cmd_hist(i) = cmd_hist(i-1)
		Enddo
		i = dialog(data).tlength
                cmd_hist(1) = dialog(data).text(1:i)//char(0)
                if (cmd_hist_len .lt. cmd_hist_max) then
                	cmd_hist_len = cmd_hist_len + 1
                endif
C
C  Run it
C
		Call execute_command(dialog(data).text(1:i))
C
C  Reset focus
C
		dialog(data).tlength = 0
		dialog(data).tpos = 0
		dialog(1).tpos = data
		running_script = 1
                cmd_hist_pos = 0
	endif
			endif
C
			Call qenter(REDRAW,wid)
		Else if (data .eq. thickedit) Then
			ftemp = rots.sthickness
			read(dialog(data).text(1:i),*,err=347,end=347)
     +				rots.sthickness
347			write(dialog(data).text,350) rots.sthickness
350	Format(F5.2)
352	Format(F7.4)
351	Format(F5.1)
			dialog(data).tlength = 5
			Call calc_up(rots,interpixel)
			ftemp = ftemp - rots.sthickness
		Else if (data .eq. ipixeledit) Then
			ftemp = rots.ipixel
			read(dialog(data).text(1:i),*,err=343,end=343)
     +				rots.ipixel
343			write(dialog(data).text,352) rots.ipixel
			dialog(data).tlength = 7
			ftemp = ftemp - rots.ipixel
		Else if (data .eq. countedit) Then
			ftemp = 0
			read(dialog(data).text(1:i),*,err=348,end=348)
     +				rots.n_slices(1)
348			write(dialog(data).text,349) rots.n_slices(1)
349	Format(I3)
			dialog(data).tlength = 3
			Call calc_up(rots,interpixel)
		Else if (data .eq. offedit) Then
			ftemp = rots.x_off
			read(dialog(data).text(1:i),*,err=346,end=346) 
     +				rots.x_off
346			write(dialog(data).text,351) rots.x_off
			dialog(data).tlength = 5
			Call calc_up(rots,interpixel)
			ftemp = ftemp - rots.x_off
		Else if (data .eq. cslewedit) Then
			ftemp = rots.slew(1)
			read(dialog(data).text(1:i),*,err=345,end=345) 
     +				rots.slew(1)
345			write(dialog(data).text,351) rots.slew(1)
			dialog(data).tlength = 5
			Call calc_up(rots,interpixel)
			ftemp = ftemp - rots.slew(1)
		Else if (data .eq. ctiltedit) Then
			ftemp = rots.tilt(1)
			read(dialog(data).text(1:i),*,err=344,end=344) 
     +				rots.tilt(1)
344			write(dialog(data).text,351) rots.tilt(1)
			dialog(data).tlength = 5
			Call calc_up(rots,interpixel)
			ftemp = ftemp - rots.tilt(1)
		Else if (data .eq. xshiftedit) Then
			ftemp = rots.xshift
			read(dialog(data).text(1:i),*,err=353,end=353) 
     +				rots.xshift
353			write(dialog(data).text,351) rots.xshift
			dialog(data).tlength = 5
			Call calc_up(rots,interpixel)
			ftemp = ftemp - rots.xshift
		Endif
		If ((autosample).and.(ftemp .ne. 0)) Then
			Call dirty_window(-1,2)
		Endif
		Call setup_dialog(rots,dialog,wid,myset,0)
		goto 10
	Endif
C
C	handle menu events
C
	if (device .eq. DLG_POPEVNT) then
		if (data .eq. 10) then  ! view popup
			i = dialog(data).tpos
C
			if ((i .ge. 1) .and. (i .le. 8)) then
C
C	Change view
C
				Do status=1,3
					rots.rots(status) = rottable(status,I)
				Enddo
				If (auto_render .eq. 1) Then
					dirty = .true.
					Call qenter(REDRAW,wid2)
				Endif
				Call setup_dialog(rots,dialog,wid,myset,wid2)
				goto 7001
C
			Else if (i .eq. 100) then  ! send 2d points to trace
C
C	convert the points to slice Zs
C
				status = 0
				Do j=1,num2dpts
C
C			If (ztoslice_t(w2dpts(j,3),2) .ne. -1) then
C
			    status=status+1
			    temppts(status,1) = w2dpts(j,1)
			    temppts(status,2) = w2dpts(j,2)
			    temppts(status,3) = w2dpts(j,3)
			    temppts(status,4) = w2dpts(j,4)

C			    temppts(status,3) = ztoslice_t(w2dpts(j,3),2)
C			Endif

				Enddo
C
C	send number of points first
C
				Call vt_write_points(status,temppts)
C
C	Let trace know!!!
C
				message(1) = IPC_POINT
				size = 1
				Call send_ipc(size,message)
C
			Else if (i .eq. 101) then  ! Reproject 2D+Z to voxel
C
C	we need a redraw
C
				Call Qenter(REDRAW,0)
C
C	This routine allows the user to send the current set of 2D+Z points
C	back through the rotation matrix to generate voxel image points
C	This is VERY useful when the voxel image rotates.
C
C	preserve cutting plane status (but we must reproject with it disabled)
C
7001				Call reproject2d3d(num2dpts,w2dpts,
     +						num3dpts,w3dpts)
				goto 10
C
C
			Else if (i .eq. 137) then  ! Movieola controls
C
				write(tstr,870) shm_data
870	Format("brainvox_voxel DIALOG_MOVIE ",I9," &")
				Call bv_system_path(tstr)
C
			Else if (i .eq. 108) then  ! Texture controls
C
				Call getarg_nq(1,tempstr)
				i = strlen(tempstr)
				write(tstr,890) tempstr(1:i),shm_data
890	Format("brainvox_voxel DIALOG_TEXTURE ",A," ",I9," &")
				Call bv_system_path(tstr)
C
			Else if (i .eq. 110) then  ! Calib Talairach
				Call wait_cursor
				Call tal_init_space(w2dpts,num2dpts,
     +					opacity,myset)
				Call arrow_cursor
				Call Qenter(REDRAW,0)

			Else if (i .eq. 136) then  ! St Louis mods to tal space
				Call tal_2_stlouis
				Call Qenter(REDRAW,0)

			Else if (i .eq. 111) then  ! Load Talairach
				Call tal_file_io("_talairach ",IPC_READ,
     +					interpixel)
				Call Qenter(REDRAW,0)

			Else if (i .eq. 112) then  ! Save Talairach
				Call tal_file_io("_talairach ",IPC_WRITE,
     +					interpixel)
			Else if (i .eq. 113) then  ! Resample Talairach MRI
				Call tal_resample("tal_res_%%%.pic ",1)
			Else if (i .eq. 131) then  ! Read _hotspots file
				Call read_tal_points(num2dpts,w2dpts,
     +					"_hotspots ")
				Call do_measure(w2dpts,num2dpts,w3dpts,
     +					num3dpts,myset,dialog,1)	
				Call qenter(REDRAW,0)
				goto 7001
			Else if (i .eq. 135) then  ! orient overlays
				label_over = 1-label_over
				if (label_over .ne. 0) Then
					wind2d(curwin).gcol = cindex
				Endif
				Call Qenter(REDRAW,0)
			Else if (i .eq. 130) then  ! TAL overlays
				tal_over = 7 - tal_over
				if (tal_over .ne. 0) gcol1 = cindex
				if (tal_over .ne. 0) Then
					wind2d(curwin).gcol = cindex
				Endif
				Call Qenter(REDRAW,0)
			Else if ((i .ge. 114).and.
     +				(i .le. 121)) then  ! Resample Talairach PET
C
C	Setup the environment (start by making a PET TAL INJ dir)
C
				pet_t_dir = " "
				Call name_changer("TAL_INJ_%%%.dir ",
     +					(i-113),pet_t_dir,error)
				Inquire(exist=error,file=pet_t_dir)
				tstr = pet_t_dir(1:
     +					strlen(pet_t_dir))//char(0)
				If (.not. error) Then
					h = '000041ff'X
					status = bv_mkdir(tstr,h)
				Endif
				tstr=" "
C
C	Get the injection template
C
				Call name_changer(inj_temp,(i-113),
     +					tstr,error)
				tempstr=" "
C
C	build the filename template from injection template
C	input:"*/"(inj)"_%%%.pic"  output:TAL_INJ_ij#.dir/(inj)"_%%%.pic"
C
				write(unit=tempstr,fmt=878) 
     +				tstr(1:strlen(tstr)),
     +				pet_t_dir(1:strlen(pet_t_dir)),
     +				tstr(1:strlen(tstr))
878	Format('tal_resample "*/',A,'_%%%.pic" "',A,'/',A,'_%%%.pic"')
				Call bv_system(tempstr)
C
C
C	Background color
C
			Else if ((i .ge. 5000).and.(i .le. 5100)) then 
C
				i = i - 5000
				i = (float(I)/100.0)*255
				bcolor = i + i*256 + i*65536
				Call setpal(palette,opacity,texture_c,
     +					texture_o,bcolor,myset)
				If (auto_render .eq. 1) Then
					dirty = .true.
					Call qenter(REDRAW,wid2)
				Endif
C
			Else if (i .eq. 132) then  ! fork off renamer
C
C	Fork off the point set renamer
C
				write(tstr,879) shm_data
879	Format("brainvox_voxel DIALOG_PSET ",I9," &")
				Call bv_system_path(tstr)
C
C	Load point set
C
			Else if ((i .ge. 3000).and.(i .le. 3100)) then 
C
			Call point_io(i-3000,tstr,0,w2dpts,num2dpts,'./ ')
				Call Qenter(REDRAW,0)
C
C		pass these read points into 3d window array 
C
				goto 7001  
C
C	Save point set
C
			Else if ((i .ge. 4000).and.(i .le. 4100)) then 
C
C	Get the pointset name
C
			Call get_point_name(i-4000,tstr,'./ ')
C	Save the points
			Call point_io(i-4000,tstr,1,w2dpts,num2dpts,'./ ')
C
C 	Toggle pointset clipping
C
			Else if (i .eq. 133) then  ! toggle clipping
				clippingon = 1 - clippingon
				Call Qenter(REDRAW,0)
			Else if (i .eq. 138) then  ! "shade" clipping
				clipshade = 1 - clipshade
				Call Qenter(REDRAW,0)
C
C fit a plane to the current points
C
			Else if (i .eq. 134) then  ! Plane fitting
C
				Call fit_cutting_plane(cindex,w2dpts,
     +					num2dpts,myset,frots)
C
C	1-error  2-area 3,4,5-rx,ry,rz 6-D
C
				If (frots(1) .eq. -1) Then
			write(*, *)"Unable to fit a plane to these points"
				Else
					wind2d(curwin).rots(1)=frots(3)
					wind2d(curwin).rots(2)=frots(4)
					wind2d(curwin).rots(3)=frots(5)
					wind2d(curwin).rots(4)=frots(6)+256
			   		Call setup_dialog(rots,dialog,
     +					wid,myset,0)
					If (autosample) then
					wind2d(curwin).dirty = .true.
						goto 10
					Endif
				Endif
C
C	Howard Dspace options
C
			Else if ((i .ge. 500) .and. (i .le. 510)) Then
				Call wait_cursor
				Call handle_dspace_menu(i,w2dpts,
     +					num2dpts,myset)
				Call arrow_cursor
				Call qenter(REDRAW,0)
C
C	RJF options
C
			Else if ((i .ge. 1000) .and. (i .le. 1100)) Then
C
C	Random points
C
				if ((i .eq. 1000) .or. (i .eq. 1001)) Then
					num2dpts = 0
					status = 200
					if (i .eq. 1001) status = 1000
					i = 0
					Do while (i .lt. status)
C	write(*, *)"looking",i
					retvals(1)=bv_frand()*imagesize
					retvals(2)=bv_frand()*imagesize
					j = (bv_frand()*realimages)+1
						j = ((j-1)*interpimages)+j
						retvals(3)=j-1
						retvals(4)=12
					j=realimages+
     +						((realimages-1)*interpimages)
					if ((retvals(1) .lt. imagesize).and.
     +					    (retvals(2) .lt. imagesize).and.
     +					    (retvals(3) .lt. j) .and.
     +					    (retvals(3) .ge. 0)) Then
C	write(*, *)"checking",retvals(1),retvals(2),retvals(3)

						Call get_pix(j,retvals,
     +				%val(images_addr),imagesize,imagesize)
						If (j .gt. 0) Then
							i = i + 1
							num2dpts = i
C	write(*, *)"adding",retvals(1),retvals(2),retvals(3),i
							Do j=1,4
						   	w2dpts(i,j)=retvals(j)
							Enddo
						Endif
					Endif
					Enddo
					Call qenter(REDRAW,0)
C
C		pass these read points into 3d window array 
C
					Goto 7001  
				Endif
			Endif
			goto 10
C
C	trace color
C
		Else if (data .eq. 13) then  ! color popup
			i = dialog(data).tpos
			if (I .ge. 0) cindex = i
			dialog(data).text = clist(cindex+1)
			Call qenter(REDRAW,wid)
			goto 10
C
C	Pointtype menu
C
		Else if (data .eq. pnttypemenu) then 
			i = dialog(data).tpos
			if (i .eq. 1) Then
				pointtype = i
				dialog(pnttypemenu).text = "Pnt:BV "
			Else if (i .eq. 2) Then
				pointtype = i
				dialog(pnttypemenu).text = "Pnt:Tal"
			Else if (i .eq. 3) Then
				pointtype = i
				dialog(pnttypemenu).text = "Pnt:Dsk"
			Endif
			Call do_measure(w2dpts,num2dpts,w3dpts,
     +				num3dpts,myset,dialog,1)	
			Call qenter(REDRAW,wid)
			Goto 10
C
C	MP_FIT opts
C
		Else if (data .eq. mpfitmenu) then  ! mpfit popup
			i = dialog(data).tpos
C
C	MP_disp : 	1 = view first slice
C			2 = view other slices
C			4 = view fixed polygon
C			8 = Overlay voxel imag?
C			16 = solid current plane
C
			if (i .eq. 1) then
				MP_disp = ieor(MP_disp,1)
				Call qenter(REDRAW,0)
			Else if (i .eq. 2) then
				MP_disp = ieor(MP_disp,2)
				Call qenter(REDRAW,0)
			Else if (i .eq. 3) then
				MP_disp = ieor(MP_disp,4)
				Call qenter(REDRAW,0)
			Else if (i .eq. 4) then
				Call save_planes(rots,myset,imagesize,
     +					interpixel,patientinfo,0)
			Else if (i .eq. 8) then
				Call save_planes(rots,myset,imagesize,
     +					interpixel,patientinfo,1)
			Else if (i .eq. 9) then
C
C	Fork off the MP_FIT file i/o interface
C
				write(tstr,877) shm_data
877	Format("brainvox_voxel DIALOG_MPIO ",I9," &")
				Call bv_system_path(tstr)

			Else if (i .eq. 5) then
				MP_disp = ieor(MP_disp,8)
				Call qenter(REDRAW,0)
			Else if (i .eq. 6) then
				MP_disp = ieor(MP_disp,16)
				Call qenter(REDRAW,0)
			Else if (i .eq. 7) then
				MP_disp = ieor(MP_disp,32)
				Call qenter(REDRAW,0)
			Else if ((i .ge. 100).and.(i .le. 114)) then
				wind2d(curwin).n_slice = i - 100
				wind2d(curwin).dirty = .true.
				Call setup_dialog(rots,dialog,wid,myset,0)
			Else if (i .eq. 10) then
C
C	Autofitting of glasses
C
				Call trace2fppoints(num2dpts,w2dpts,rots)
				Call  calc_fp(rots)
				Call  calc_up(rots,interpixel)
				If (autosample) Then
					Call dirty_window(-1,2)
				Endif
				Call qenter(REDRAW,0)
			Else if (i .eq. 11) then
C Load
				Call glasses_io(IPC_READ,rots)
				Call  calc_fp(rots)
				Call  calc_up(rots,interpixel)
				If (autosample) Then
					Call dirty_window(-1,2)
				Endif
				Call qenter(REDRAW,0)
			Else if (i .eq. 12) then
C Save
				Call glasses_io(IPC_WRITE,rots)
			Endif
			goto 10
C
C	point menus
C
		Else if ((data .ge. pointmenus).and.
     +			(data .le. pointmenus+3)) then  ! point popups
			i = dialog(data).tpos
			if (i .le. 0) goto 10
			Call getroipnt(i,frots)
			rots.fppoints(data-pointmenus+1,1) = frots(1)
			rots.fppoints(data-pointmenus+1,2) = frots(2)
			rots.fppoints(data-pointmenus+1,3) = frots(3)
			Call  calc_fp(rots)
			Call  calc_up(rots,interpixel)
			If (autosample) Then
				Call dirty_window(-1,2)
			Endif
			Call qenter(REDRAW,0)
			goto 10
C
C	measure color
C
		Else if (data .eq. measurecolor) then  ! color popup
			i = dialog(data).tpos
			If (i .lt. 100) then
C
C	Change the color and
C
				dialog(data).text = clist(i+1)
				linedata.color = i
C
C	recompute the measurements
C
				Call do_measure(w2dpts,num2dpts,w3dpts,
     +					num3dpts,myset,dialog,1)	
			Else if (i .eq. 100) then
C
C	recompute and output a FLIP surface file
C
				Call do_measure(w2dpts,num2dpts,w3dpts,
     +					num3dpts,myset,dialog,2)	
			Else if (i .eq. 101) then
C
C	recompute and output a SURFVIEW surface file
C
				Call do_measure(w2dpts,num2dpts,w3dpts,
     +					num3dpts,myset,dialog,3)	
			Else if (i .eq. 102) then
C
C	Run SURFVIEW on the file Voxel.surf
C
				Call bv_system_path(
     +					"surfview -s Voxel.surf &")

			Else if (i .eq. 103) then
C
C	Run SURFVIEW (flip mode) on the file Voxel.surf
C
				Call bv_system_path(
     +					"surfview -f -s Voxel.surf &")
			Endif
C
			Call qenter(REDRAW,0)
			goto 10
C
C	preview
C
		Else If (data .eq. 12) then   !preview popup
			i = dialog(data).tpos
			If (i .eq. 100) Then
C
C	textured rendering
C
				use_16bit = 1-use_16bit
                                If ((use_hardware .ne. 0) .or.
     +                              (auto_render .ne. 0)) Then
				   dirty = .true.
				   Call qenter(REDRAW,wid2)
                                Endif
			Else if (i .eq. 101) Then
C
C	overlay render
C
				over_render = 1
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Else if (i .eq. 103) Then
C
C	Auto render
C
                                auto_render = 1-auto_render
				If (auto_render .eq. 1) Then
					dirty = .true.
					Call qenter(REDRAW,wid2)
				Endif
			Else if (i .eq. 104) Then
C
C	Force a reload of the HW textures
C
				Call vl_dirty(%val(2),myset)
				If (auto_render .eq. 1) Then
					dirty = .true.
					Call qenter(REDRAW,wid2)
				Endif
			Else if (i .eq. 102) Then
C
C	hardware render
C
                                use_hardware = 1-use_hardware
				dirty = .true.
				Call qenter(REDRAW,wid2)
C
			Else if (i .eq. 602) then  ! Median Filter
C
				Call wait_cursor
				Call do_filter_proc(%val(save_ptr),
     +					%val(zbufptr),%val(image3dsize),
     +					%val(image3dsize),%val(10))
				Call arrow_cursor
				Call Qenter(REDRAW,wid2)
C
			Else if (i .eq. 603) then  ! Average Filter
C
				Call wait_cursor
				Call do_filter_proc(%val(save_ptr),
     +					%val(zbufptr),%val(image3dsize),
     +					%val(image3dsize),%val(11))
				Call arrow_cursor
				Call Qenter(REDRAW,wid2)
C
			Else if (i .eq. 604) then  ! ZAverage Filter
C
				Call wait_cursor
				Call do_filter_proc(%val(save_ptr),
     +					%val(zbufptr),%val(image3dsize),
     +					%val(image3dsize),%val(15))
C
C	recompute the measurements
C
		Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +			myset,dialog,1)	
C
				Call arrow_cursor
				Call Qenter(REDRAW,0)
C
			Else if (i .eq. 605) then  ! ZMedian Filter
C
				Call wait_cursor
				Call do_filter_proc(%val(save_ptr),
     +					%val(zbufptr),%val(image3dsize),
     +					%val(image3dsize),%val(16))
C
C	recompute the measurements
C
		Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +			myset,dialog,1)	
C
				Call arrow_cursor
				Call Qenter(REDRAW,0)
C
			Else if (i .eq. 139) then  ! Lighting
C
				Call wait_cursor
				norm(1) = 0.0
				norm(2) = 0.0
				norm(3) = 1.0
				norm(4) = 1.0
				Call do_postlight(%val(save_ptr),
     +					%val(zbufptr),
     +					%val(save_ptr),
     +					%val(image3dsize),
     +					%val(image3dsize),
     +					norm)
				Call arrow_cursor
				Call Qenter(REDRAW,wid2)
C
			Else if (i .eq. 140) then  ! Lighting controls
C
				write(tstr,880) shm_data
880	Format("brainvox_voxel DIALOG_LMOD ",I9," &")
				Call bv_system_path(tstr)
C
			Else if (i .ge. 1) then
				Call vl_speed(%val(i),myset)
				If (auto_render .eq. 1) Then
					dirty = .true.
					Call qenter(REDRAW,wid2)
				Endif
			Endif
			goto 10 
		Else If (data .eq. 14) then   !clear 100=all 101=image 102=vox
			i = dialog(data).tpos
			if (i .eq. 101) then
				num2dpts = 0
			Else if (i .eq. 102) then
				num3dpts = 0
			Else if (i .eq. 100) then
				num3dpts = 0
				num2dpts = 0
			Else if ((i .ge. 0) .and. (i .le. 13)) then
				Call delpoints(num2dpts,w2dpts,i)
				Call delpoints(num3dpts,w3dpts,i)
			Endif
C
C	recompute the measurements
C
			Call do_measure(w2dpts,num2dpts,w3dpts,num3dpts,
     +				myset,dialog,1)	
C
			Call qenter(REDRAW,0)   
			goto 10 
C
C	Cutting state (slices,arbitrary,MP_FIT)
C
		Else if (data .eq. statemenu) then  ! changing cutting state
			i = dialog(data).tpos
			if (i .eq. 0) then
				wind2d(curwin).mode = WIN_SLICE_MODE
				wind2d(curwin).dirty = .true.
			Else if (i .eq. 1) then
				wind2d(curwin).mode = WIN_ARB_MODE
				wind2d(curwin).dirty = .true.
			Else if (i .eq. 2) then
				wind2d(curwin).mode = WIN_MPFIT_MODE
				call calc_fp(rots)
				call calc_up(rots,interpixel)
			Else if (i .eq. 1000) then
				autosample = .not. autosample
			if (autosample) wind2d(curwin).dirty = .true.
			Else if (i .ge. 9101) Then
				Call sel_window(i - 9100,1,myset,rots)
			Endif
			Call setup_dialog(rots,dialog,wid,myset,0)
			goto 10 
		Endif
		goto 10
	Endif
C
C	handle scrollbar events
C
	if (device .eq. DLG_BAREVNT) then
		status = 0
		i = dialog(data).aux
		if (data .eq. 15) then
			wind2d(curwin).inum2d = i
			wind2d(curwin).dirty = .true.
			Call qenter(REDRAW,wind2d(curwin).wid)
			Call qenter(REDRAW,wid)
		Else if ((data .ge. cutbars).and.(data .le. cutbars+3)) then
C
C	arbitrary mode
C
			If (wind2d(curwin).mode .eq. WIN_ARB_MODE) Then
				wind2d(curwin).rots(data-cutbars+1) = i
				i = wind2d(curwin).wid
			Endif
C
C	MP_FIT mode  (Slew,Tilt,TableH,TableV)
C
			If (wind2d(curwin).mode .eq. WIN_MPFIT_MODE) then
				if (data .eq. cutbars+1) 
     +					rots.slew(2)=(float(i)/2.0)-90.0
				if (data .eq. cutbars+0) 
     +					rots.tilt(2)=(float(i)/2.0)-90.0
				if (data .eq. cutbars+3) 
     +					rots.uptrans(2) = i - 256.0
				if (data .eq. cutbars+2) 
     +					rots.uptrans(3) = i - 180.0
				Call calc_up(rots,interpixel)
				If (autosample) Then
					Call dirty_window(-1,2)
				Endif
				i = 0
			Endif
C
C	Check for autosample
C
			Call setup_dialog(rots,dialog,wid,myset,i)
			If (autosample) then
				wind2d(curwin).dirty = .true.
				goto 10
			Endif
		Else
C
C	Cube rotations
C
			rots.rots(data-5) = i
			status = 1
			If (auto_render .eq. 1) Then
				dirty = .true.
				Call qenter(REDRAW,wid2)
			Endif
		Endif
		Call setup_dialog(rots,dialog,wid,myset,0)
		if (status .eq. 0) goto 10
C
C	reproject the points and return to top of loop
C
		goto 7001
C
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
	Call winclo(wid2)
	Do i=1,MAX_WINDOWS
		If (wind2d(i).wid .ne. -1) Then
			Call winclo(wind2d(i).wid)
		Endif
	Enddo
	Call vt_quit_shmem
	Call bv_exit(0)
C
	End
C
C  Send a filename as a message
C
	Subroutine      send_string(string,tag_code,len)

	Implicit None

	Include '../libs/vt_inc.f'

	Integer*4       string(*)
	Integer*4       tag_code,len,i,j
	Integer*4       message(IPC_SHM_SIZE)
C
C       Setup the header
C
	i = 1
	message(i) = IPC_F_RETURN
	i = i + 1
	message(i) = tag_code
	Do j = 1,(len+5)/4
	i = i + 1
	message(i) = string(j)
	Enddo
	Call send_ipc(i,message)
C
	Return
	End

