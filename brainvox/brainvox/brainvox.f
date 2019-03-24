	Program Brainvox

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'vtuif.inc'
	Include 'prefs.f'

	logical		error

	Integer*4	lastid   ! last image type loaded (0=whole 1=rh 2=lh)
	Integer*4	status,state,i
	Integer*4	device,cur_sel
	Integer*4	wid,x,y,noreaper
	Integer*2	data
	character*200	tstr
	character*200	roi,rgn
	Record /ditem/	dialog(100)
	Integer*4	svol_menu

	integer*4	iargc,signal,bv_fork,bv_getpid,strlen

	external 	iargc,signal,bv_fork,bv_getpid,strlen
C
C	initially we have no images loaded
C
	lastid = -1
	cut_dir = lastid
	svol_menu = 0
	cur_sel = 1
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
C	Do we have a new cwd?
C
	If (iargc() .ne. 0) Then
		Call getarg(1,tstr)
		If (tstr .ne. '-e') Then
			i = strlen(tstr)
			If (index(tstr,"_._patient") .ne. 0) Then
				i = index(tstr,"_._patient") - 2
			Else If (index(tstr,"_patient") .ne. 0) Then
				i = index(tstr,"_patient") - 2
			Endif
			write(*,*) "Start dir:",tstr(1:i)
			Call bv_chdir(tstr(1:i)//char(0))
		Endif
	Endif
C
C	print version and program ID
C
	If (iargc() .eq. 0) then

	write(*,1) "Brainvox and PET-Brainvox",version,com_date
1	Format(" Welcome to ",A," version:",A15,/,
     +	       " Copyright (C) ",A15," University of Iowa,",
     +         " Department of Neurology",/,
     +         "               ","               ",
     +	       " University of Iowa,"," Image Analysis Facility")
	write(*,2)
2	Format(/,"This program was designed by:",/,
     +	 "Hanna Damasio, Department of Neurology, ",
     +	 "Univ of Iowa, College of Medicine",/,
     +	 "Randall Frank, Department of Neurology, ",
     +	 "Univ of Iowa, College of Medicine",/,
     +	 "Thomas Grabowski, Department of Neurology, ",
     +   "Univ of Iowa, College of Medicine",/
     +	 )

	Endif
C
C	Attempt to read the _patient file
C
	volumedepth = 1  ! This is currently not in the _patient file
	volume_lims(1) = 0
	volume_lims(2) = 255
	volume_zswap = 0
	volume_bswap = 0
	Call read_patient("_patient ",error)
	If (error) then
C
C	fill in some defaults and let the user enter them
C
		realimages = 1
		imagesize = 256
		interpimages = 3
		numrgns = 0
		imagename = "_%%%.pic"
		patientinfo(1) = "Patient info line 1"
		patientinfo(2) = "Patient info line 2"
		patientinfo(3) = "Patient info line 3"
		interpixel = 1.0
		interslice = 1.0
		squeezefactor = 1.0
		volumedepth = 1
		volume_zswap = 0
		volume_bswap = 0
		volume_lims(1) = 0
		volume_lims(2) = 255
		state = 2
	Else
		state = 1
	Endif
	If (realimages .lt. 1) state = 2  ! if no images can be found
C
C	Ask the user about setting up a new dataset
C
	If (error) Then
		Call new_dataset(i)
		If (i .eq. 0) Call bv_exit(0)
	Endif
C
C	make the basic files
C
	Call create_defaults
C
C	if first command line parameters is '-e' then we are in the edit state
C
	If (iargc() .ne. 0) Then
		Call getarg(1,tstr)
		If (tstr .eq. '-e') Then
			state = 2
		Endif
	Endif
C
C	Get the environmental variable options
C
	Call bv_get_options
	If (bv_test_option(OPTS_TESTING)) Then
		write(*,*) "Brainvox - Testing mode activated."
	Endif
	If (bv_test_option(OPTS_ADVANCED)) Then
		write(*,*) "Brainvox - Advanced mode activated."
	Endif
	If (bv_test_option(OPTS_RJF)) Then
		write(*,*) "Brainvox - Greetings, Randy. "//
     +			"How about a nice game of chess?"
	Endif
C
C	I am the shared memory owner!!! (there can be only one!!)
C
	shmem_owner = 1
	Call vt_init_shmem
C
C	get the window
C
C Call prefsi(700,235)	! open 700x235 window
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-235)/2
	call prefpo(x,x+700,y,y+235)

	write(tstr,512) version,""
512	Format("Brainvox ",A,A,"}")
	wid=winope(tstr,index(tstr,"}")-1)
	call rgbmod
	call doubleb
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('vtuif.r',3-state,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
	call setup_dialog(state,dialog,svol_menu,cur_sel)
C
	Call get_pref_i(PREF_NOREAPER,noreaper)
	If (noreaper .ne. 0) Then
		write(*,*) "Brainvox - not using shmem reaper"
	Else
		If (bv_fork() .eq. 0) Then
C
C	Start the shared memory reaper...
C
			shmem_owner = 0
			Call vt_quit_shmem
			Call brainvox_reaper(%val(shm_ipc),%val(shm_data),
     +				%val(shm_images),%val(shm_pts),
     +				%val(shm_texture))
		Endif
	Endif
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,wid)	! force a redraw
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! end of program
C		goto 999    
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) goto 999     
		Call swapbu
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) goto 999    
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	Handle popup menus
C
	If (device .eq. DLG_POPEVNT) Then
		status = dialog(data).tpos
		If (status .lt. 0) goto 10
		If (state .eq. 1) Then
			If (data .eq. bv_svol_menu) Then
				cur_sel = status
				Call indexnum_to_roi(status,roi,i)
				write(tstr,12) roi(1:strlen(roi))
12	Format("Select subvolume:",A,"}")
				dialog(bv_svol_menu).text = 
     +					tstr(1:index(tstr,"}")-1)
				dialog(bv_svol_menu).tlength = 
     +					index(tstr,"}")-1
				Call qenter(REDRAW,wid)	! force a redraw
			Endif
		Else
			If (data .eq. bv_depth_menu) Then
				volumedepth = status
		dialog(bv_depth_menu).text = "Image type:8bit "
		If (volumedepth .eq. 1) Then
			dialog(bv_min_edit).dtype = DLG_NOP
			dialog(bv_max_edit).dtype = DLG_NOP
		Else
			dialog(bv_min_edit).dtype = DLG_EDIT
			dialog(bv_max_edit).dtype = DLG_EDIT
		Endif
		If (volumedepth .eq. 2) Then
			dialog(bv_depth_menu).text = "Image type:16bit"
		Else if (volumedepth .eq. 4) Then
			dialog(bv_depth_menu).text = "Image type:float"
		Endif
				Call qenter(REDRAW,wid)	! force a redraw
			Endif
		Endif
	Endif
C
C	handle buttons
C
	If (device .eq.  DLG_CHKEVNT) then
		If (state .eq. 2) then
			If (data .eq. bv_zswap_chk) then
				volume_zswap = dialog(bv_zswap_chk).aux
			Endif
			If (data .eq. bv_bswap_chk) then
				volume_bswap = dialog(bv_bswap_chk).aux
			Endif
		Endif
	Endif
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		If (state .eq. 1) then
			If (data .eq. 7) then   !quit
				goto 999
			Else if (data .eq. 8) then !edit info
				goto 998
			Else if (data .eq. bv_preferences) then !preferences
				tstr = "brainvox_prefs &"
				Call bv_system_path(tstr)
			Else if (data .eq. 13) then !trace hemis
C
C	load images if needed
C
			  if (lastid .ne. 0) then
				lastid = 0
				cut_dir = 0
				call loadimages(error)
				if (error) goto 10
			  Endif
			  call sublaunch("brainvox_trace _hemis -s }",
     +				wid,"_hemis }")
			Else if (data .eq. 14) then !edit rgn lists
C
C	start the region editor
C
			  call sublaunch("brainvox_regions }",wid," }")
			  call setup_dialog(state,dialog,svol_menu,cur_sel)
C
			Else if (data .eq. bv_svol_button) then !edt subvol rgns
C
C	load images if needed
C
			  cut_dir = cur_sel
			  If (lastid .ne. cut_dir) then
				lastid = cut_dir
				call loadimages(error)
				if (error) goto 10
			  Endif
			  Call indexnum_to_roi(cut_dir,roi,i)
			  Call roi_to_rgn(roi,rgn)
			  write(tstr,100) rgn(1:strlen(rgn))
100	Format("brainvox_trace '",A,"' -s }")
			  Call sublaunch(tstr,wid,
     +				rgn(1:strlen(rgn))//" }")
C
			Else if (data .eq. 17) then !calc stats 
C
C	start the calculation program
C
			  call sublaunch("brainvox_calc }",wid," }")
			Endif
			goto 10
		Else
			If (data .eq. 7) then   ! Continue
				goto 998
			Else if (data .eq. 8) then ! Abort    
				goto 999
			Endif
		Endif
	Endif
C
C	handle edit text
C
	if (device .eq. DLG_EDTEVNT) then
		i = dialog(data).tlength
		if (data .eq. 9) then
			patientinfo(1) = dialog(data).text(1:i)
		Else if (data .eq. 10) then
			patientinfo(2) = dialog(data).text(1:i)
		Else if (data .eq. 11) then
			patientinfo(3) = dialog(data).text(1:i)
		Else if (data .eq. 12) then
			imagename = dialog(data).text(1:i)
		Else if (data .eq. 13) then
			read(dialog(data).text(1:i),*,end=600,err=600)
     +				 interpixel
600			write(dialog(data).text,599) interpixel
			dialog(data).tlength = 10
599	format(f10.5)
			call qenter(REDRAW,wid)
		Else if (data .eq. 14) then
			read(dialog(data).text(1:i),*,end=601,err=601)
     +				 interslice
601			write(dialog(data).text,599) interslice
			dialog(data).tlength = 10
			call qenter(REDRAW,wid)
		Else if ((data .eq. bv_min_edit).or.(data .eq. 
     +			bv_max_edit)) then
			read(dialog(data).text(1:i),*,end=602,err=602)
     +				volume_lims(data-bv_min_edit+1)
602			write(dialog(data).text,599) 
     +				volume_lims(data-bv_min_edit+1)
			dialog(data).tlength = 10
			call qenter(REDRAW,wid)
		Endif
	Endif
C
	Goto 10
998	continue
C
C	handle a system state change
C
	call winclo(wid)
	if (state .eq. 2) then
C
C	create default pals and regions (and create the sub dirs)
C	(this happens only if necessary)
C
		Call create_defaults
		Call write_patient("_patient ",error)
		If (error) Then
			Call bv_error("Unable to save _patient file"//
     +			"|Check file permissions and disk space}",1)
		Endif
		tstr = "brainvox &"   
	else
		tstr = "brainvox -e &"
	endif
	Call vt_quit_shmem
	Call bv_system_path(tstr)
	Call bv_exit(0)
C
C	quit routine  (no saving )
C
999	continue
	Call vt_quit_shmem
	call winclo(wid)
	Call bv_exit(0)

	End
C
C	routine to fill out dialogs
C
	subroutine setup_dialog(state,dialog,svol_menu,cur_sel)

	Implicit None

	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'vtuif.inc'

	logical		error

	Integer*4	status,state,i,j
	Integer*4	strlen,svol_menu,cur_sel
	Character*200	tstr,roi

	Record /ditem/	dialog(*)

	external	strlen
C
C	free up any existing popup menu
C
	If (svol_menu .ne. 0) Call freepu(svol_menu)
	svol_menu = 0
C
C	state number
C
	if (state .eq. 1) then
		dialog(9).text = patientinfo(1) 
		dialog(10).text = patientinfo(2) 
		dialog(11).text = patientinfo(3) 
		dialog(9).tlength = strlen(patientinfo(1)) 
		dialog(10).tlength = strlen(patientinfo(2)) 
		dialog(11).tlength = strlen(patientinfo(3)) 

		dialog(12).text = imagename
		dialog(12).tlength = strlen(imagename)

		svol_menu = newpup()
		dialog(bv_svol_menu).aux = svol_menu
		dialog(bv_svol_menu).tpos = 1
		cur_sel = 1
C
C	Build the menu
C
		j = 1
		Call indexnum_to_roi(j,roi,i)
		Call clean_roi_name(roi)
		Do while (i .ne. -1)
			If (j .eq. cur_sel) Then
				write(tstr,9) roi(1:strlen(roi)),j
			Else
				write(tstr,11) roi(1:strlen(roi)),j
			Endif
9	Format(A," %r1%I%x",I2.2,"}")
11	Format(A," %r1%i%x",I2.2,"}")
			Call addtop(svol_menu,tstr,index(tstr,"}")-1,0)
			j = j + 1
			Call indexnum_to_roi(j,roi,i)
			Call clean_roi_name(roi)
		Enddo
		Call indexnum_to_roi(cur_sel,roi,i)
		write(tstr,12) roi(1:index(roi," ")-1)
12	Format("Select subvolume:",A,"}")
		dialog(bv_svol_menu).text = tstr(1:index(tstr,"}")-1)
		dialog(bv_svol_menu).tlength = index(tstr,"}")-1
	Else 
		dialog(9).text = patientinfo(1) 
		dialog(10).text = patientinfo(2) 
		dialog(11).text = patientinfo(3) 
		dialog(9).tlength = strlen(patientinfo(1)) 
		dialog(10).tlength = strlen(patientinfo(2)) 
		dialog(11).tlength = strlen(patientinfo(3)) 

		dialog(12).text = imagename
		dialog(12).tlength = strlen(imagename)
	
		write(dialog(13).text,10) interpixel
		dialog(13).tlength = 10
10	format(f10.5)
		write(dialog(14).text,10) interslice
		dialog(14).tlength = 10

		dialog(bv_depth_menu).aux = newpup()
		dialog(bv_depth_menu).tpos = 1
		tstr = "8bit %x1|16bit %x2|Float %x4}"
		Call addtop(dialog(bv_depth_menu).aux,tstr,
     +			index(tstr,"}")-1,0)

		dialog(bv_depth_menu).text = "Image type:8bit "
		If (volumedepth .eq. 1) Then
			dialog(bv_min_edit).dtype = DLG_NOP
			dialog(bv_max_edit).dtype = DLG_NOP
		Else
			dialog(bv_min_edit).dtype = DLG_EDIT
			dialog(bv_max_edit).dtype = DLG_EDIT
		Endif
		If (volumedepth .eq. 2) Then
			dialog(bv_depth_menu).text = "Image type:16bit"
		Else if (volumedepth .eq. 4) Then
			dialog(bv_depth_menu).text = "Image type:float"
		Endif

		write(dialog(bv_min_edit).text,10) volume_lims(1)
		dialog(bv_min_edit).tlength = 10
		write(dialog(bv_max_edit).text,10) volume_lims(2)
		dialog(bv_max_edit).tlength = 10
	
		dialog(bv_zswap_chk).aux = volume_zswap
		dialog(bv_bswap_chk).aux = volume_bswap

	Endif

	return
	End
C
C
C
	subroutine	sublaunch(str,wid,rgn)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'

	Integer*4	wid,x,y
	Character*(*)	str,rgn
	Character*12	id
	Character*200	tstr
	Character*200	tstr2
	Character*200	tstr3

	integer*4	strlen

	external strlen
C
C	update the shared memory data block
C
	Call vt_write_data
	Call init_ipc(1,%val(ipc_addr))
C
C	hide our window
C
	Call winclo(wid)
	Call bv_sginap(2000) ! delay 2 sec for shmem update
C
C	spawn off palette editor and voxel processor
C	when the trace application is launched
C
	if (index(str,"brainvox_trace") .ne. 0) then
		write(unit=id,fmt=10) shm_data
		tstr = "brainvox_pals "//id//" &"
		call bv_system_path(tstr)
		tstr2 = "brainvox_voxel '"//rgn(1:index(rgn,"}")-1)
     +			//"'"//id//" &"
		call bv_system_path(tstr2)
	Endif
C
C
C	launch the subprogram  (and wait for it to exit)
C
	write(unit=id,fmt=10) shm_data
10	format(I10)
	tstr3 = str(1:index(str,"}")-1)//" "//id
	call bv_system_path(tstr3)
C
C	wait for an IPC_QUIT code, then
C	wait for all child processes to detach
C
	Call vt_child_wait
C
C	restore our window
C
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-235)/2
	call prefpo(x,x+700,y,y+235)
	write(tstr,512) version,""
512	Format("Brainvox ",A,A,"}")
	wid=winope(tstr,index(tstr,"}")-1)
	call rgbmod
	Call doubleb
	Call gconfi
C
C	fix the queue
C
	Call qreset
	Call qenter(REDRAW,wid)
C
	return
	end
C
C	this routine creates default directories/pals/regions
C	(if they do not exist already)
C
	subroutine create_defaults

	Implicit None

	Include '../libs/vt_inc.f'

	integer*4	i,mode
	Integer*4	pal(768)
	character*30	tstr
	logical		error
C	
C	init the palette
C
	do i=0,255
		pal(i+1) = i
		pal(i+257) = i
		pal(i+513) = i
	enddo
C
C	check for existance and then write
C	the luts
C	
	inquire(exist=error,file="_ilut")
	if (.not. error) call write_pal("_ilut ",pal,error)
	inquire(exist=error,file="_vlut")
	if (.not. error) call write_pal("_vlut ",pal,error)
	inquire(exist=error,file="_opacity")
	if (.not. error) call write_pal("_opacity ",pal,error)
	inquire(exist=error,file="_texture")
	if (.not. error) call write_pal("_texture ",pal,error)
	inquire(exist=error,file="_texopac")
	if (.not. error) call write_pal("_texopac ",pal,error)

	do i=0,255
		pal(i+1) = 255
		pal(i+257) = 255
		pal(i+513) = 255
	enddo

	inquire(exist=error,file="_iopac")
	if (.not. error) call write_pal("_iopac ",pal,error)
C
C	hemis  (Kathy Jones likes them in this order...)
C
	numrgns = 3
	rgnnames(1) = "*whole.roi"
	rgnnames(2) = "*lefthemi.roi"
	rgnnames(3) = "*righthemi.roi"
	rgntypes(1) = 1
	rgntypes(2) = 1
	rgntypes(3) = 1
	inquire(exist=error,file="_hemis")
	if (.not. error) then
		call write_rgns("_hemis ",error)
	Endif
C
	Call build_subdirs
C
	Call read_rgns("_hemis ",error)
C
	return	
	end
C
	Subroutine build_subdirs()

	Implicit None

	Include '../libs/vt_inc.f'

	integer*4	i,bv_mkdir,mode,heminum,j,strlen
	logical		error
	Character*200	dir,tstr,roi

	external 	bv_mkdir,strlen
C
	mode = '000041ff'X
C
	Call read_rgns("_hemis ",error)
C
	Do i=1,numrgns
C
C	Get the directory name
C
		Call roi_to_dir(rgnnames(i),dir)
C
		Inquire(exist=error,file=dir)
		tstr = dir(1:strlen(dir))//char(0)
		If (.not. error) j = bv_mkdir(tstr,mode)
C
	Enddo
C
C	Now check that the appropriate .rgns files exist
C 
	heminum = numrgns  ! must hold onto this as we will be changing it
C
	Do i=1,heminum
C
C	Get each roi template from _hemis and convert to .rgns filename
C
		Call indexnum_to_roi(i,roi,j)
		Call roi_to_rgn(roi,tstr)
C
C	Build a fake initial .rgns file
C
		numrgns = 1
		rgnnames(1) = roi
		rgntypes(1) = 1
		inquire(exist=error,file=tstr)
		if (.not. error) then
C
C	and write it if needed
C
			call write_rgns(tstr,error)
		Endif
C
	Enddo
C
	Return
	End
C
C	routine to read defaults via file pointed to be ENV var
C
	Subroutine  readenvdefs(which)

	Implicit None

	Include '../libs/vt_inc.f'

	Character*(*) 	which
	Character*150	filename
	Character*200	temp,temp2

	Integer*4	strlen
	External	strlen
C
C	attempt to get ENV variable
C
        Call getenv('BRAINVOX_DEFRGNS',filename)    
C
	open(66,file=filename,form='formatted',readonly,
     +             status='old',err=999)
C
C	read the file
C
	temp = "_-_-_"
	Do while (index(temp,which) .eq. 0)
		Read(66,10,end=999,err=999) temp
10	Format(A200)
	Enddo
C
C	Read pairs of lines (until .)
C
100	continue
	read(66,10,err=999,end=999) temp
	if (temp(1:1) .eq. '.') goto 999
	read(66,10,err=999,end=999) temp2
	if (temp2(1:1) .eq. '.') goto 999
	numrgns = numrgns + 1
	rgnnames(numrgns) = temp(1:strlen(temp))
	rgntypes(numrgns) = 1
	if (index(temp2,"arc") .ne. 0) then
		rgntypes(numrgns) = 2
	Else if (index(temp2,"intersect") .ne. 0) then
		rgntypes(numrgns) = 2
	Else if (index(temp2,"lesion") .ne. 0) then
		rgntypes(numrgns) = 3
	Endif
	goto 100
C
C	close the file and exit
C
999	close(66)
C
	Return
	End
C
C	Ask the user if a new dataset should be set-up
C
	Subroutine new_dataset(setup)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'vtuif.inc'
	Include '../libs/vt_inc.f'

	Integer*4	setup
	Record /ditem/	dialog(100)
	Integer*4	status,i
	Integer*4	device
	Integer*4	wid,x,y
	Integer*2	data
	Character*100	tstr

	setup = 1
C
C	get the window
C
C 	Call prefsi(700,235)	! open 700x235 window
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-235)/2
	call prefpo(x,x+700,y,y+235)

	write(tstr,512) version,""
512	Format("Brainvox ",A,A,"}")
	wid=winope(tstr,index(tstr,"}")-1)
	call rgbmod
	call doubleb
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('vtuif.r',3,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,wid)	! force a redraw
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! end of program
C		goto 999    
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) goto 999     
		Call swapbu
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) goto 999    
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	Dialog events are now in the queue
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		If (data .eq. bv_new_setup) Then
			goto 888
		Else if (data .eq. bv_new_exit) Then
			goto 999
		Endif
	Endif
C
C	top of event loop
C
	goto 10
C
C	Return with new setup
C
888	Call winclo(wid)
	setup = 1
	Return
C
C	Return and exit
C
999	Call winclo(wid)
	setup = 0
	Return

	End
