	Program brainvox_map3

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'map3uif.inc'
	Include 'map3.inc'
	Include '../filer/filer_inc.inc'

	logical		error

	Integer*4	status,i,j
	Integer*4	device
	Integer*4	wid,x,y,h
	Integer*2	data
	Real*4		v
C
	Record /ditem/	dialog(100)
C
	Record /MAP3/	map
C
	Record /slist/	rlist
	Record /slist/	llist
C
	Character*256	cwd,orig_cwd,cwd_base
	Integer*4	cwd_type  ! 0=normal dir 1=_patient level 2=rois
	Integer*4	mode ! 0=normal 1=graph
	Logical		found
C
	Character*256	tstr
C
	Character*200	last_set,last_values
C
	Common	/work/last_set,last_values
C
C       IPC defines
C
        Integer*4       size,message(IPC_SHM_SIZE)
C
	integer*4	iargc,strlen,bv_chdir,bv_getcwd

	external 	iargc,strlen,bv_chdir,bv_getcwd
C
	last_set = "<none> "
	last_values = "<none> "
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
C	print version and program ID
C
	If ((iargc() .lt. 1).or.(bv_test_option(OPTS_TESTING))) Then
	write(*,1)version,com_date
1	Format(" Welcome to BRAINVOX_REGIONS version:",A15,/,
     +	       " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
	Endif
C
	if (iargc() .lt. 1) then
2		write(*, 17)
17	Format("This program must be launched from within brainvox.") 
		Call bv_exit(1)
	Endif
	call getarg_nq(iargc(),tstr)
	read(unit=tstr,fmt=*,err=2) shm_data
C
C	I am NOT the shared memory owner!!! (there can be only one!!)
C
	shmem_owner = 0
	Call vt_init_shmem
C
C	initial setup
C
	i = bv_getcwd(orig_cwd,%val(255))
	If (i .eq. 0) Then
		write(*,*)"Warning, unable to determine the ",
     +			"current working directory"
	Endif
C
	Call getenv("BRAINVOX_MAP3_BASE",cwd)
	If (cwd(1:1) .eq. ' ') Then
		cwd = "/usr/czone/PC_for_lesion_analysis/MAP-3"
	Endif
	cwd_type = cwd_home
	cwd_base = cwd
C
C	Check for a bad setup
C
	tstr = cwd(1:strlen(cwd))//"/_patient"
	Inquire(file=tstr,exist=found)
        If (found) Then
		write(*,*)"Warning: the MAP3 directory contains"//
     +				" an _patient file"
		write(*,*)"That configuration may cause problems"
		write(*,*)"See:",tstr
	Endif
C
C
	tstr = cwd_base(1:strlen(cwd_base))//char(0)
	i = bv_chdir(tstr)
	If (i .ne. 0) Then
		write(*,*)"Warning, unable to set the ",
     +			"current working directory to the MAP-3 base:"
		write(*,*)cwd_base(1:strlen(cwd_base))
	Endif
C
C	Null MAP3 and lists
C
	map.num = 0 
	map.exp = 0.0
	map.offset = 0.0
	map.n_image = 0
	map.size_weight = sw_none

	rlist.select = 0
	rlist.top = 0
	llist.select = 0
	llist.top = 0
C
C	get the window
C
	mode = 0
	call prefsi(600,285)
	wid=winope("MAP-3",5)
	Call rgbmod
	Call doubleb
	Call gconfi
C
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('map3uif.r',1,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
C	menus
C
	dialog(map3_opts).aux = newpup()
	tstr = "MAP-3 options %t|No size weighting %I%x1%r1}"
	Call addtop(dialog(map3_opts).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Linear size weighting %i%x2%r1}"
	Call addtop(dialog(map3_opts).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Negative linear size weighting %i%x3%r1%l}"
	Call addtop(dialog(map3_opts).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Reset values to 1.0 %x4}"
	Call addtop(dialog(map3_opts).aux,tstr,index(tstr,"}")-1,0)
C
	Call new_map3(map,rlist)
	Call new_cwd(oper_init,cwd,cwd_type,llist)
	Call setup_dialog(llist,rlist,map,cwd,cwd_type,dialog,1)
C
C	IPC stuff
C
        Call qdevic(MOUSE2)
        Call qdevic(TIMER0)
        Call noise(TIMER0,20)  !every 1/3 seconds
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,wid)	! force a redraw
C
10	device=qread(data)	! top of event loop
C
C       timer stuff
C
        if (device .eq. TIMER0) then
                size = IPC_SHM_SIZE
                call check_ipc(size,message)
                If (message(1) .eq. IPC_QUIT) Then
                        goto 999
		Else if (message(1) .eq. IPC_F_RETURN) Then
			If (message(2) .eq. FLR_MAP3_LOAD_SET) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
					Call MAP3_settings_IO(tstr,
     +						IPC_READ,i,map)
					Call new_map3(map,rlist)
					last_values = "<none> "
					Call set_name(last_set,tstr)
				Endif
			Else if (message(2) .eq. FLR_MAP3_SAVE_SET) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
					Call MAP3_settings_IO(tstr,
     +						IPC_WRITE,i,map)
				Endif
			Else if (message(2) .eq. FLR_MAP3_LOAD_VALS) Then
				Call get_filename(tstr,message(3),i)
				If (i .ne. 0) Then
					Call MAP3_external_values(
     +						tstr,map)
					Call new_map3(map,rlist)
					Call set_name(last_values,tstr)
				Endif
			Endif 
			Call setup_dialog(llist,rlist,map,cwd,
     +				cwd_type,dialog,1)
		Else
			goto 10
		Endif
	Endif
C
C	Redraws...
C
	If (device .eq. REDRAW) then	! redraw screen
		If (mode .eq. 0) Then
			Call RESHAP	! do not forget !!!
			Call DRAW_DLG(dialog,status)
			If (status .ne. 0) goto 999     
			Call swapbu
		Else 
			Call RESHAP	! do not forget !!!
			h = '00545454'X
			Call cpack(h)
			Call Clear
			Call draw_plot(map)
			Call Swapbu
		Endif
		Goto 10
	Endif
C
C	Handle mode 1
C
	If (mode .ne. 0) Then
		If ((device .eq. MOUSE1).and.(data .eq. 1)) Then
			mode = 0
			Call qenter(REDRAW,wid)
		Endif
		goto 10
	Endif
C
C	Handle real dialog input
C
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) Then
		write(*, *)"Status error:",status
		goto 999    
	Endif
	If (device .eq.  DLG_NOPEVNT) goto 10
C
C	handle menus
C
	If (device .eq. DLG_POPEVNT) Then
		status = dialog(data).tpos
		If (status .lt. 1) goto 10
		If (data .eq. map3_opts) Then
			If (status .eq. 1) Then
				map.size_weight = sw_none
			Else if (status .eq. 2) Then
				map.size_weight = sw_linear
			Else if (status .eq. 3) Then
				map.size_weight = sw_invlin
			Else if (status .eq. 4) Then
				Do i=1,map.num
					map.value(i) = 1.0
				Enddo
				Call new_map3(map,rlist)
				last_values = "<none> "
				Call setup_dialog(llist,rlist,map,cwd,
     +					cwd_type,dialog,0)
			Endif
		Endif
		goto 10
	Endif
C
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		If (data .eq. map3_done) then   !quit
C
C	Done using MAP3
C
			goto 999
C
		Else if (data .eq. map3_plot) then
C
C	plot mode
C
			mode = 1
			Call qenter(REDRAW,wid)
			goto 10
C
		Else if (data .eq. map3_add) then
C
C	Add current ROI to map params
C
			If ((map.num .lt. max_rois) .and.
     +				(llist.select .gt. 0)) Then
				i = llist.select
				map.num = map.num + 1
				map.dir(map.num) = cwd
				map.roi(map.num) = llist.names(i)
				map.arb_mode(map.num) = llist.tag(i)
				map.value(map.num) = 1.0
C
C	Sort the MAP3 list (bubble sort)
C
				Do i=1,map.num-1
					Do j=i+1,map.num
						If (map.roi(i) .gt. 
     +							map.roi(j)) Then
					tstr = map.roi(i)
					map.roi(i) = map.roi(j)
					map.roi(j) = tstr

					tstr = map.dir(i)
					map.dir(i) = map.dir(j)
					map.dir(j) = tstr

					v = map.value(i)
					map.value(i) = map.value(j)
					map.value(j) = v

					status = map.arb_mode(i)
					map.arb_mode(i) = map.arb_mode(j)
					map.arb_mode(j) = status

						Endif
					Enddo
				Enddo
C
C	update the display
C
				Call new_map3(map,rlist)
			Else
				goto 10
			Endif
		Else if (data .eq. map3_remove) then
C
C	Remove current ROI from map params
C
			If (rlist.select .gt. 0) Then
				Do i=rlist.select,map.num-1
					map.dir(i) = map.dir(i+1)
					map.roi(i) = map.roi(i+1)
					map.value(i) = map.value(i+1)
					map.arb_mode(i) = map.arb_mode(i+1)
				Enddo
				map.num = map.num - 1
				Call new_map3(map,rlist)
			Else 
				goto 10
			Endif
		Else if (data .eq. map3_clear) then
C
C	Clear the current map params
C
			map.num = 0 
			Call new_map3(map,rlist)
			last_set = "<none> "
			last_values = "<none> "
C
		Else if (data .eq. map3_compute) then
C
C	Compute the MAP3
C
C	CD to initial cwd
C
			i = bv_chdir(orig_cwd)
C
C	Run the paint program
C
			Call compute_map3(map)
C
C	Let the world know there is a new texture volume
C
			Call send_ipc(1,IPC_TEXVOLUME)
C
C	CD back to the MAP-3 base dir
C
			tstr = cwd_base(1:strlen(cwd_base))//char(0)
			i = bv_chdir(tstr)
C
			Call qenter(REDRAW,wid)
C
		Else if ((data .eq. map3_load) .or.
     +			(data .eq. map3_save) .or.
     +			(data .eq. map3_file_values)) then
C
C	filer services
C
1000     Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A," &")
1010     Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A,
     +          " -s ",A," &")
C
C	Fork off the filer
C
			If (data .eq. map3_save) Then
			    write(tstr,1010) shm_data,FLR_MAP3_SAVE_SET,
     +				   '"Save MAP-3 setup as... (*.m3set)"',
     +				   '"*.m3set"',"untitled.m3set"
			Else if (data .eq. map3_load) Then
			    write(tstr,1000) shm_data,FLR_MAP3_LOAD_SET,
     +				   '"Load MAP-3 setup... (*.m3set)"',
     +				   '"*.m3set"'
			Else
			    write(tstr,1000) shm_data,FLR_MAP3_LOAD_VALS,
     +				   '"Load external values... (*.txt)"',
     +				   '"*.txt"'
			Endif
			Call bv_system_path(tstr)
C
		Else if (data .eq. map3_down) then
C
C	Down
C
			If ((llist.select .eq. 1) .and. 
     +				(llist.names(1) .eq. "/..")) Then
				Call new_cwd(oper_up,cwd,cwd_type,llist)   
			Else
				Call new_cwd(oper_down,cwd,cwd_type,llist)   
			Endif
		Else if (data .eq. map3_up) then
			Call new_cwd(oper_up,cwd,cwd_type,llist)   
		Else if (data .eq. map3_home) then
			Call new_cwd(oper_home,cwd,cwd_type,llist)   
		Endif
		Call setup_dialog(llist,rlist,map,cwd,cwd_type,dialog,0)
		goto 10
	Endif
C
C	handle edit text
C
	if (device .eq. DLG_EDTEVNT) then
		i = dialog(data).tlength
		tstr=dialog(data).text
		read(unit=tstr,fmt=*,err=100,end=100) v
100		If (data .eq. map3_edit_value) then
			If (rlist.select .ge. 1) Then
				if (v .lt. 0.0) v = 0
				if (v .gt. 1.0) v = 1.0
				map.value(rlist.select) = v
			Endif
			Call new_map3(map,rlist)   
		Else if (data .eq. map3_exp) then
			map.exp = v
		Else if (data .eq. map3_offset) then
			map.offset = v
		Endif
		Call setup_dialog(llist,rlist,map,cwd,cwd_type,
     +				dialog,1)
		goto 10
	Endif
C
C	handle scrollbar
C
	if (device .eq. DLG_BAREVNT) then
		i = dialog(data).aux + 1 
		If (data .eq. map3_rscroll) then
			rlist.top = i
		Else if (data .eq. map3_lscroll) then
			llist.top = i
		Endif
		Call setup_dialog(llist,rlist,map,cwd,cwd_type,dialog,0)
		goto 10
	Endif
C
C	checkbox events
C
	if (device .eq. DLG_CHKEVNT) then
		if (data .eq. map3_n_chk) then  
			i = dialog(data).aux
			If (i .eq. 1) Then
				map.n_image = 1
			Else 
				map.n_image = 0
			Endif
		Endif
		goto 10
	Endif
C
C	handle clickable text
C
	if (device .eq. DLG_TXTEVNT) then
		If (data .ge. map3_rtext) Then
			i = data-map3_rtext+rlist.top
			rlist.select = i
			write(tstr,200) map.value(i)
200	Format(F5.2)
			dialog(map3_edit_value).text=tstr
			dialog(map3_edit_value).tlength = 5
			dialog(map3_edit_value).tpos = 5
		Else
			i = data-map3_ltext+llist.top
C
C	double click
C
			If (i .eq. llist.select) Then
			    If ((cwd_type .eq. cwd_rois).and.
     +					(i.gt.1)) Then
				Call Qenter(DLG_BUTEVNT,map3_add)
			    Else if (llist.names(i) .eq. "/..") Then
				Call new_cwd(oper_up,cwd,cwd_type,llist)   
			    Else
				Call new_cwd(oper_down,cwd,cwd_type,llist)   
			    Endif
			Else
C
C	single click
C
				llist.select = i
			Endif
		Endif
		Call setup_dialog(llist,rlist,map,cwd,cwd_type,dialog,0)
		goto 10
	Endif
C
	Goto 10
C
C	quit routine  (no saving )
C
999	continue
C
C	cut the umbilical
C
	call winclo(wid)
	Call vt_quit_shmem
	Call exit(0)

	End
C
C	build rightlist
C
	Subroutine new_map3(map,inlist)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'map3uif.inc'
	Include 'map3.inc'

	Record  /map3/ map
	Record	/slist/ inlist

	Integer*4	i
	Character*200	tstr,tstr2
C
	Do i=1,map.num
		tstr2 = map.roi(i)
		If (map.arb_mode(i) .eq. 1) Then
			write(unit=tstr,fmt=20) tstr2(1:16),map.value(i)
		Else 
			write(unit=tstr,fmt=10) tstr2(1:19),map.value(i)
		Endif
		inlist.names(i) = tstr(1:40)
	Enddo
	inlist.num = map.num
C
10	Format(A19,F5.2)
20	Format(A16,"ARB",F5.2)
C
C	correct current selection
C
	If (inlist.select .gt. inlist.num) inlist.select = inlist.num
C
C	correct list top
C	
	If (inlist.top .lt. 1) inlist.top = 1
	If (inlist.top .gt. inlist.num) inlist.top = inlist.num
C
	Return
	End
C
C	Read values for a new cwd
C
	Subroutine new_cwd(oper,cwd,cwd_type,inlist)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'map3uif.inc'
	Include 'map3.inc'
	Include '../trace/vt_arb_inc.f'

	Record	/slist/	inlist
	Character*256	cwd
	Integer*4	cwd_type  
	Integer*4	oper

	Integer*4	strlen,bv_exists
	external 	strlen,bv_exists

	Integer*4	i,j,k,ptr,num

	Logical		found,error
	Byte            buffer(max_names*100)
	Character*100	fnames(max_names)

        Integer*2       compar
        External        compar

	Character*256	base_cwd,tstr,tstr2

	common /new_cwd_save/ base_cwd
C
	Call wait_cursor
C
C	perform the requested operation
C
	If (oper .eq. oper_init) Then
		base_cwd = cwd
		cwd_type = cwd_home
	Else if (oper .eq. oper_home) Then
		cwd = base_cwd
		cwd_type = cwd_home
		inlist.select = 0
	Else if (oper .eq. oper_reread) Then
	Else if (oper .eq. oper_up) Then
		i = strlen(cwd)
		Do while(i .gt. 1)
			If (cwd(i:i) .ne. '/') Then
				cwd(i:i) = ' '
			Else
				cwd(i:i) = ' '
				goto 100
			Endif
			i = i - 1
		Enddo
100		Continue
		If (strlen(cwd) .le. strlen(base_cwd)) Then
			cwd = base_cwd
			cwd_type = cwd_home
		Else
			cwd_type = -1
		Endif
		inlist.select = 0
		inlist.top = 0
	Else if (oper .eq. oper_down) Then
		If (inlist.select .ge. 1) Then
			i = strlen(cwd)
			cwd = cwd(1:i)//inlist.names(inlist.select)
			cwd_type = -1
		Endif
		inlist.select = 0
		inlist.top = 0
	Endif
C
C	get new cwd type if unknown
C
	If (cwd_type .eq. -1) Then
C
C	Assume
C
		cwd_type = cwd_dir
C
C	look for _patient
C
		i=strlen(cwd)
		tstr = cwd(1:i)//"/_patient"
          	If (bv_exists(tstr) .ne. 0) cwd_type = cwd_pat_dir
C		Inquire(file=tstr,exist=found)
C		If (found) cwd_type = cwd_pat_dir
C
C	look for ../_patient
C
		tstr = cwd(1:i)//"/../_patient"
          	If (bv_exists(tstr) .ne. 0) cwd_type = cwd_rois
C		Inquire(file=tstr,exist=found)
C		If (found) cwd_type = cwd_rois
	Endif
C
C	Read the current dir (based on type)
C
	inlist.num = 0
	If (cwd_type .eq. cwd_rois) Then
C
C	"UP" name
C
		num = 1
		fnames(1) = "/.."
C
C	get the .rlst filename
C
		tstr = cwd
		i=strlen(tstr)
		Do while((i .gt. 1).and.(tstr(i:i) .ne. "/"))
			tstr(i:i) = " "
			i = i - 1
		Enddo
		j = index(cwd,".dir")
		If (j .gt. i) Then
			tstr2 = "_"//cwd(i+1:j)//"rlst"
			tstr = tstr(1:i)//tstr2
C
C	read the rgn names and place into the fnames array
C
			Call read_rgns(tstr,error)
			If (.not. error) Then
				Do i=2,numrgns  !first rgn is the subvolume
					num = num + 1
					fnames(num) = rgnnames(i)
				Enddo
			Endif
		Endif
	Else
C
C       call C-routine defined in sgiuif package
C
		tstr = cwd(1:strlen(cwd))//char(0)
        	j = max_names*100
        	i = 0
        	Call read_filenames(tstr,buffer,%val(j),%val(0),i,%val(0))
C
C	Parse out the names
C
		num = 0
        	ptr=1
        	Do j=1,i
                	tstr=""
                	k=0
                	Do while (buffer(ptr) .ne. 0)
                        	k=k+1
                        	tstr(k:k)=char(buffer(ptr))
                        	ptr=ptr+1
                	Enddo
                	ptr=ptr+1
			fnames(num+1) = tstr(1:k)//" "
C
C       (save none of the '.' files...)
C
                	If ((num .lt. max_names-1 ) .and.
     +                  	(fnames(num+1)(1:1) .eq. "/") .and.
     +                          (fnames(num+1) .ne. "/.")) then
                        	num=num+1
				if ((fnames(num) .eq. "/..").and.
     +					(cwd_type .eq. cwd_home)) num=num-1
                	Endif
        	Enddo
C
	Endif
C
	If (num .gt. 1) Then
C
C       sort the array
C
		If (fnames(1) .eq. "/..") Then
       			Call qsort(fnames(2),num-1,100,compar)
		Else 
       			Call qsort(fnames(1),num,100,compar)
		Endif
	Endif
C
C	which rois are in ARB mode
C
	Do i=1,num
		inlist.tag(i) = 0
		inlist.names(i) = fnames(i)
	Enddo
	If (cwd_type .eq. cwd_rois) Then
C
C	Read the "Trace_Slicing.tag" file
C
		tstr=cwd(1:strlen(cwd))//"/Trace_Slicing.tag"//char(0)
		Call Tagfile_Arb_IO_file(tstr,IPC_READ,error)
		If (error) Then
			write(*,3)"Warning:Unable to read the tagfile",
     +			tstr(1:strlen(tstr))
		Else
			Do i=2,num
C
C	If the match is (1), then there is no specific ARB mode for this ROI
C
				Call roiname_to_arbmode(fnames(i),j)
				If (j .eq. 1) Then
					inlist.tag(i) = 0
				Else
					inlist.tag(i) = 1
				Endif
			Enddo
		Endif
3		Format(A,A)
	Endif
C
C	fix up the selection
C
	inlist.num = num
	If (inlist.select .gt. inlist.num) inlist.select = inlist.num
	If (inlist.top .lt. 1) inlist.top = 1
	If (inlist.top .gt. inlist.num) inlist.top = inlist.num
C
	Call arrow_cursor
C
	Return
	End
C
C	routine to fill out dialogs
C
	subroutine setup_dialog(llist,rlist,map,cwd,cwd_type,dialog,edt)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'map3uif.inc'
	Include 'map3.inc'
C
	Character*256	cwd
	Integer*4	cwd_type  
	Integer*4	edt
	Record	/slist/	rlist
	Record	/slist/	llist
	Record  /MAP3/	map
C
	Character*200	tstr
C
	logical		error

	Integer*4	status,i,j
	Integer*4	strlen

	Record /ditem/	dialog(*)

	external	strlen
C
	Character*200	last_set,last_values
C
	Common	/work/last_set,last_values
C
C	setup the scroll bars
C
	dialog(map3_rscroll).aux = rlist.top-1
	dialog(map3_rscroll).tlength = rlist.num-1

	dialog(map3_lscroll).aux = llist.top-1
	dialog(map3_lscroll).tlength = llist.num-1
C
C	now the scrolling lists
C
	Do I=0,9
		dialog(i+map3_rtext).text = " "
		dialog(i+map3_rtext).tlength = 1
		dialog(i+map3_rtext).aux = 4  ! left/nonhighlighted
		dialog(i+map3_rtext).tpos = 0 ! non-clickable

		j = rlist.top+i
		if ((j .le. rlist.num).and.(rlist.num .gt. 0)
     +			.and.(rlist.top .gt. 0)) then
			tstr = rlist.names(j)
			dialog(i+map3_rtext).text=tstr(1:24)
			dialog(i+map3_rtext).tlength=24
			If (rlist.select .eq. j) Then
				dialog(i+map3_rtext).aux = 5 !left/light
			Endif
			dialog(i+map3_rtext).tpos = 1 ! clickable
		Endif

		dialog(i+map3_ltext).text = " "
		dialog(i+map3_ltext).tlength = 1
		dialog(i+map3_ltext).aux = 4  ! left/nonhighlighted
		dialog(i+map3_ltext).tpos = 0 ! non-clickable

		j = llist.top+i
		if ((j .le. llist.num).and.(llist.num .gt. 0)
     +			.and.(llist.top .gt. 0)) then
			tstr = llist.names(j)
			If (llist.tag(j) .eq. 1) Then
				dialog(i+map3_ltext).text=tstr(1:21)//"ARB"
			Else
				dialog(i+map3_ltext).text=tstr(1:24)
			Endif
			dialog(i+map3_ltext).tlength=24
			If (llist.select .eq. j) Then
				dialog(i+map3_ltext).aux = 5 !left/light
			Endif
			dialog(i+map3_ltext).tpos = 1 ! clickable
		Endif
	Enddo
C
C	check on add/remove buttons
C
	dialog(map3_add).dtype = DLG_NOP
	dialog(map3_remove).dtype = DLG_NOP
	dialog(map3_clear).dtype = DLG_NOP
	dialog(map3_up).dtype = DLG_NOP
	dialog(map3_down).dtype = DLG_NOP
	dialog(map3_home).dtype = DLG_NOP
	dialog(map3_save).dtype = DLG_NOP
	dialog(map3_compute).dtype = DLG_NOP

	If ((llist.select .gt. 1).and.(cwd_type .eq. cwd_rois)) Then
		 dialog(map3_add).dtype=DLG_BUTTON
	Endif
	If (rlist.select .gt. 0) dialog(map3_remove).dtype=DLG_BUTTON
	If (rlist.num .gt. 0) dialog(map3_clear).dtype=DLG_BUTTON
	If (cwd_type .ne. cwd_home) dialog(map3_up).dtype=DLG_BUTTON
	If (cwd_type .ne. cwd_home) dialog(map3_home).dtype=DLG_BUTTON
	If ((cwd_type .ne. cwd_rois) .and. (llist.select .gt. 0)) Then
		dialog(map3_down).dtype=DLG_BUTTON
	Endif
	If (map.num .gt. 0) dialog(map3_save).dtype=DLG_BUTTON
	If (map.num .gt. 0) dialog(map3_compute).dtype=DLG_BUTTON
C
C	Edit fields
C
	If (edt) Then
		If (rlist.select .gt. 0) Then
			i = rlist.select
			write(dialog(map3_edit_value).text,10)map.value(i)
			dialog(map3_edit_value).tpos = 5 
			dialog(map3_edit_value).tlength = 5 
		Endif

		write(dialog(map3_exp).text,11) map.exp
		dialog(map3_exp).tpos = 6
		dialog(map3_exp).tlength = 6

		write(dialog(map3_offset).text,11) map.offset
		dialog(map3_offset).tpos = 6
		dialog(map3_offset).tlength = 6
10	Format(F6.2)
11	Format(F6.2)
	Endif
C
C	Setup checkbox (could come externally)
C
	dialog(map3_n_chk).aux = map.n_image
C
	write(dialog(map3_files).text,12) last_set(1:strlen(last_set)),
     +		last_values(1:strlen(last_values))
12	Format("Files:",A,":",A,"}")
	dialog(map3_files).tlength = index(dialog(map3_files).text,"}")-1
C
C	redraw
C
	Call qenter(REDRAW,0)
C
	return
	End
C
C	Routine to fixup an ROIname
C
	Subroutine fixup_roiname(name)

	Implicit None

	Character*(*)	name
	
	Integer*4	i,strlen
	External strlen
C
C	If no ".suffix" then add ".roi"
C
	If ((index(name,".roi").eq. 0).and.
     +		(index(name,".rgn").eq. 0)) Then
		i = strlen(name)
		name = name(1:i)//".roi"
	Endif
C
C	if no "*" or "%" prefix with "*"
C
	If ((index(name,"*").eq.0).and.(index(name,"%").eq.0)) Then
		i = strlen(name)
		name = "*"//name(1:i)
	Endif
C
	Return
	End
C
C-------------comparison function---------------
C
        Integer*2       function        compar(f1,f2)

        Implicit None

        Character*100   f1,f2
C
C       return  0 if f1=f2
C       return -1 if f1<f2  (f1 is before f2)
C       return  1 if f2<f1  (f2 is before f1)
C
        compar=0
C
        If (f1 .eq. f2) return
C
        If (llt(f1,f2)) Then  !is f1<f2
                compar=-1
        Else
                compar=1
        Endif
C
        return
        end
C
C	subroutine to draw the weighting function plot
C

C#define INSET 20

	Subroutine draw_plot(map)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'map3uif.inc'
	Include 'map3.inc'

	Record	/MAP3/	map
	Integer*4	i,j
	Integer*4	wx,wy,h
	Real*4		p1(2),p2(2)
	Real*4		fx,fy
	Character*256	tstr

	Integer*4	INSET
	Parameter	(INSET = 20)
C
C	setup
C
	h = '00000303'X
	Call deflin(2768,h)

	Call getsiz(wx,wy)
	wx = wx - 2*INSET
	wy = wy - 2*INSET
C
C	Grid
C
	Call cpack(0)
	fx = float(wx)/10.0
	fy = float(wy)/4.0
	Call setlin(2768)
	Do i=1,9
		Call bgnlin
		p1(1) = INSET + i*fx
		p1(2) = INSET
		Call v2f(p1)
		p2(1) = INSET + i*fx
		p2(2) = INSET + wy
		Call v2f(p2)
		Call endlin
	Enddo
	Do i=1,3
		Call bgnlin
		p1(1) = INSET 
		p1(2) = INSET + i*fy
		Call v2f(p1)
		p2(1) = INSET + wx
		p2(2) = INSET + i*fy
		Call v2f(p2)
		Call endlin
	Enddo
C
C	box
C
	Call setlin(0)
	h = '00ffffff'X
	Call cpack(h)
	Call recti(INSET,INSET,INSET+wx,INSET+wy)
C
C	Function
C
	h = '000000ff'X
	Call cpack(h)
	Do i=0,500 
		fx = float(i)/500.0
		fy = 0.1+0.9/(1.0+exp(map.exp*(fx+map.offset)))
		If ((map.exp .eq. 0).and.(map.offset .eq. 0)) fy=1.0
		p1(1) = p2(1)
		p1(2) = p2(2)
		p2(1) = INSET + (fx*wx)
		p2(2) = INSET + (fy*wy)
		If (i .gt. 0) Then
			Call bgnlin
			Call v2f(p1)
			Call v2f(p2)
			Call endlin
		Endif
	Enddo
C
C	Function
C
	h = '00ffffff'X
	Call cpack(h)
C
	write(tstr,10) map.exp,map.offset
10	Format("y=0.1+0.9/(1+exp(",F6.2,"*(x+",F6.2,")))}")
C
	Call cmov2i(INSET,4)
C
	If ((map.exp .eq. 0).and.(map.offset .eq. 0)) Then
		tstr = "y=1.0}"
	Endif
C
	i = index(tstr,"}")-1
	Call charst(tstr,i)
C
C	Title
C
	tstr="Performance weighting function:}"
C
	Call cmov2i(INSET,INSET+wy+4)
C
	i = index(tstr,"}")-1
	Call charst(tstr,i)
C
	Return
	End
C
C
C
	Subroutine set_name(out,in)

	Implicit None

	Character*(*)	out,in
	Integer*4	i,j,strlen
	External	strlen

	i = strlen(in)
	If (i .le. 1) Return
	j = i-1
	Do while (j .ge. 1)
		If (in(j:j) .eq. "/") Then
			out = in(j+1:i)
			Return
		Endif
		j = j - 1
	Enddo
	out = in(1:i)
	
	Return
	End
