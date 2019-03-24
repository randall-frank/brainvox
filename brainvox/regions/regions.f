	Program brainvox_regions

*#define MAX_SVOLS max_rgns

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'regionsuif.inc'

	logical		error

	Integer*4	status,i,top,side,j
	Integer*4	device
	Integer*4	wid,x,y
	Integer*2	data
	character*100	tstr,edtname,tsvolname,rgn,dir
	integer*4	edttype
	Record /ditem/	dialog(100)

	Integer*4	MAX_SVOLS
	Parameter	(MAX_SVOLS = max_rgns)
C
	Character*40	rgns(MAX_SVOLS,max_rgns)
	Integer*4	types(MAX_SVOLS,max_rgns)
	Integer*4	nums(MAX_SVOLS)
	Integer*4	h
C
	Integer*4	svol_menu
C
	integer*4	iargc,strlen,bv_mkdir,bv_rename,bv_unlink,bv_rmdir

	external 	iargc,strlen,bv_mkdir,bv_rename,bv_unlink,bv_rmdir

	common	/rgnstuff/edttype,edtname,tsvolname
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
	svol_menu = 0
C
C	read in the regions
C
	Call read_rgns("_hemis ",error)
	Call rgnio(IPC_READ,nums,types,rgns)
C
C	get the window
C
	Call foregr
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-230)/2
	call prefpo(x,x+700,y,y+230)
	wid=winope("Regions",7)
	Call rgbmod
	Call doubleb
	Call gconfi
C
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('regionsuif.r',1,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
	top = 1
	edttype = 1
	edtname = ""
	tsvolname = ""
	side = 1    ! 1=right 2=left,...
	call setup_dialog(top,side,nums,types,rgns,dialog,svol_menu)
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,wid)	! force a redraw
10	device=qread(data)	! top of event loop
C	If (device .eq. WINQUI) then	! end of program
C		goto 999    
C	Endif
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
C	In case corrupted files are seen, fix them
C
	If ((edttype .ne. 1) .and. (edttype .ne. 3)) edttype = 1
C
C	handle menus
C
	If (device .eq. DLG_POPEVNT) Then
		status = dialog(data).tpos
		If (status .lt. 1) goto 10
		If (data .eq. bv_svol_menu) Then
			If (svol_menu .ne. 0) Call freepu(svol_menu)
			svol_menu = 0
			side = status
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
		Endif
	Endif
C
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		If (data .eq. 12) then   !quit
			goto 999
		Else if (data .eq. 11) then !remove region
			if (nums(side) .le. 1) goto 10 ! may not remove hemis
			i = dialog(6).tlength
			edtname = dialog(6).text(1:i)
C
C	find a match
C
			status = 0
			do i=2,nums(side)
				if (rgns(side,i) .eq. edtname) status=i
			enddo
			if (status .ne. 0) then
C
C	remove one by shifting down
C
				do i=status,nums(side)-1
					rgns(side,i) = rgns(side,i+1)
					types(side,i) = types(side,i+1)
				enddo
				nums(side) = nums(side)-1
			Endif
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
		Else if (data .eq. 10) then !add region
			if (nums(side) .ge. max_rgns) goto 10
			i = dialog(6).tlength
			edtname = dialog(6).text(1:i)
			nums(side) = nums(side)+1
			rgns(side,nums(side)) = edtname
			types(side,nums(side)) = edttype
			Call fixup_roiname(rgns(side,nums(side)))
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
C
C	Subvolume buttons
C
		Else if (data .eq. bv_remove_svol) Then
C
C	get the rgn file name
C
			Call roi_to_rgn(rgnnames(side),rgn)
			Call roi_to_dir(rgnnames(side),dir)
C
C	Remove rgn file and subdir (ifempty)
C
			i = strlen(dir)
			i = bv_rmdir(dir(1:i)//char(0))
			i = strlen(rgn)
			i = bv_unlink(rgn(1:i)//char(0))
C
C	Remove the entry from _hemis list
C
			Do i=side,numrgns
C
				rgnnames(i) = rgnnames(i+1)
				rgntypes(i) = rgntypes(i+1)
C
C	And the memory loaded lists
C
				nums(i) = nums(i+1)
				Do j=1,max_rgns
					rgns(i,j) = rgns(i+1,j)
					types(i,j) = types(i+1,j)
				Enddo
C
			Enddo
			numrgns = numrgns - 1
C
C			writeout the _hemis file
C
			Call write_rgns("_hemis ",error)
			if (error) Call bv_error(
     +  			"Unable to write to _hemis file|"//
     +				"Check file permissions or disk space}",0)
C
C	new current selection
C
			If (svol_menu .ne. 0) Call freepu(svol_menu)
			svol_menu = 0
			side = side - 1
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
C
		Else if (data .eq. bv_add_svol) Then
C
C	Check for existing subvolume name match
C
			status = 0
			write(tsvolname,138) status
			j = 1
			Do while (j .eq. 1)
				j = 0
				Do i=1,numrgns
				If (rgnnames(i).eq.tsvolname) Then
					j = 1
				Endif
				Enddo
				If (j .eq. 1) Then
					write(tsvolname,138) status
138	Format("*subvolume",I2.2,".roi")
					status = status + 1
				Endif
			Enddo
C
C	add to _hemis list
C
			numrgns = numrgns + 1
			rgnnames(numrgns) = tsvolname
			rgntypes(numrgns) = 1
C
C			writeout the _hemis file
C
			Call write_rgns("_hemis ",error)
			if (error) Call bv_error(
     +  			"Unable to write to _hemis file|"//
     +				"Check file permissions or disk space}",0)
C
C	add to the memory list
C
			nums(numrgns) = 1
			rgns(numrgns,1) = tsvolname
			types(numrgns,1) = 1
C
C	Create subdir
C
			Call roi_to_dir(rgnnames(numrgns),dir)
			i = strlen(dir)
			h = '000041ff'X
			i = bv_mkdir(dir(1:i)//char(0),h)
C
C	new .rlst file is created on exit
C
C
C	new current selection
C
			If (svol_menu .ne. 0) Call freepu(svol_menu)
			svol_menu = 0
			side = numrgns
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
C
		Else if (data .eq. bv_rename_svol) Then
C
			i = dialog(bv_edit_svol).tlength
			If (i .lt. 1) Then
				Call ringbe
				goto 10
			Endif
			tsvolname = dialog(bv_edit_svol).text(1:i)
			Call fixup_roiname(tsvolname)
C
C	Must change the name...
C
			If (rgnnames(side) .eq. tsvolname) Then
				Call ringbe
				goto 10
			Endif
C
C	rename the directory
C
			Call roi_to_dir(rgnnames(side),dir)
			Call roi_to_dir(tsvolname,rgn)
			i = strlen(dir)
			j = strlen(rgn)
			i = bv_rename(dir(1:i)//char(0),rgn(1:j)//char(0))
C
C	remove the old .rlst file
C
			Call roi_to_rgn(rgnnames(side),rgn)
			i = strlen(rgn)
			i = bv_unlink(rgn(1:i)//char(0))
C
C	change _hemis data and record
C
			rgnnames(side) = tsvolname
			Call write_rgns("_hemis ",error)
			if (error) Call bv_error(
     +  			"Unable to write to _hemis file|"//
     +				"Check file permissions or disk space}",0)
C
C	Change the in-memory first roiname (written out automatically on exit)
C
			rgns(side,1) = rgnnames(side)
C
C	new current selection
C
			If (svol_menu .ne. 0) Call freepu(svol_menu)
			svol_menu = 0
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
C
		Else if (data .eq. bv_rgn_sort) Then
C
C	Sort (case insensitive) the region names (bubble sort)
C       The first name stays fixed 
C
			Do i=2,nums(side)-1
				Do j=i+1,nums(side)
					If (rgns(side,j) .lt. 
     +						rgns(side,i)) Then
C
C	swap
C
					    tstr=rgns(side,j)
					    rgns(side,j)=rgns(side,i)
					    rgns(side,i)=tstr
					    status=types(side,j)
					    types(side,j)=types(side,i)
					    types(side,i)=status
					Endif
				Enddo
			Enddo
C
C	new current selection
C
			If (svol_menu .ne. 0) Call freepu(svol_menu)
			svol_menu = 0
			call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
C
		Endif
		goto 10
	Endif
C
C	handle edit text
C
	if (device .eq. DLG_EDTEVNT) then
		i = dialog(data).tlength
		If (data .eq. bv_edit_svol) Then
			tsvolname = dialog(data).text(1:i)
		Else
			edtname = dialog(data).text(1:i)
		Endif
	Endif
C
C	handle scrollbar
C
	if (device .eq. DLG_BAREVNT) then
		i = dialog(data).aux + 1  ! 0-nums(side)-1
		if (i .gt. nums(side)) i = nums(side)
		top = i
		call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
		goto 10
	Endif
C
C	checkbox events
C
	if (device .eq. DLG_CHKEVNT) then
		if (data .eq. 9) then  ! lesion type
			i = dialog(data).aux
			If (i .eq. 1) Then
				edttype = 3
			Else 
				edttype = 1
			Endif
		Endif
		goto 10
	Endif
C
C	handle clickable text
C
	if (device .eq. DLG_TXTEVNT) then
		i = data - 17 + top
		if ((i.gt.nums(side)).or.(i .lt. 1)) goto 10
		edtname = rgns(side,i)      
		edttype = types(side,i)     
		dialog(6).text=edtname
		dialog(6).tlength=strlen(dialog(6).text)
		if (dialog(6).tlength .lt. 1) dialog(6).tlength=1
		dialog(6).tpos = dialog(6).tlength
		call setup_dialog(top,side,nums,types,rgns,dialog,
     +				svol_menu)
		goto 10
	Endif
C
	Goto 10
C
C	quit routine  (no saving )
C
999	continue
C
C	save the results
C
	Call rgnio(IPC_WRITE,nums,types,rgns)
C
C	tell mom (any any children) I'm done
C
	Call send_ipc(1,IPC_QUIT)
C
C	cut the umbilical
C
	call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)

	End
C
C	routine to fill out dialogs
C
	subroutine setup_dialog(top,side,nums,types,rgns,dialog,svol_menu)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'regionsuif.inc'
C
	Character*100	edtname,tsvolname
	Integer*4	edttype   !1=ROI 3=LESION
C
	Integer*4	MAX_SVOLS
	Parameter	(MAX_SVOLS = max_rgns)
C
	Character*40	rgns(MAX_SVOLS,max_rgns)
	Integer*4	types(MAX_SVOLS,max_rgns)
	Integer*4	nums(MAX_SVOLS)
C
	Integer*4	svol_menu
	Character*200	roi,tstr
C
	logical		error

	Integer*4	status,top,side,i,j
	Integer*4	strlen

	Record /ditem/	dialog(*)

	external	strlen

	common	/rgnstuff/edttype,edtname,tsvolname
C
	If (svol_menu .eq. 0) Then
C
C	make sure we are up to date
C
		Call read_rgns("_hemis ",error)
C
		svol_menu = newpup()
		dialog(bv_svol_menu).aux = svol_menu
C
C       Build the menu
C
		roi = " "
		Do i=1,numrgns
			roi = rgnnames(i)
                        Call clean_roi_name(roi)
			status = strlen(roi)
			if (i .eq. side) Then
                        	write(tstr,9) roi(1:status),i
9       Format(A," %r1%I%x",I2.2,"}")
			Else
                        	write(tstr,7) roi(1:status),i
7       Format(A," %r1%i%x",I2.2,"}")
			Endif
                        Call addtop(svol_menu,tstr,index(tstr,"}")-1,0)
C
C	Update the subvolume label
C
			If (i .eq. side) Then
				dialog(bv_edit_svol).text=roi(1:status)
				dialog(bv_edit_svol).tlength=status
				dialog(bv_edit_svol).tpos=status
				tsvolname = roi(1:status)
			Endif
                Enddo
	Endif
C
C	setup the scroll bar
C
	if (top .gt. nums(side)) top = nums(side)
	dialog(13).aux = top-1
	dialog(13).tlength = nums(side)-1
C
C	setup the radio buttons
C
	dialog(7).aux = 0
	dialog(8).aux = 0
	dialog(9).aux = 0
	dialog(4).aux = 0
	dialog(5).aux = 0
C
	dialog(9).aux = 0
	if (edttype .eq. 3) dialog(9).aux = 1
C
C	now the scrolling list
C
	Do I=17,22
		dialog(i).text = ""
		dialog(i).tlength = 1
		dialog(i).aux = 4  ! left/nonhighlighted
		dialog(i).tpos = 0 ! non-clickable
		if ((top+(i-17) .le. nums(side)).and.
     +			(nums(side) .gt. 0).and.(top .gt. 0)) then
			dialog(i).text = rgns(side,(top+(i-17)))
			dialog(i).tlength = strlen(dialog(i).text)
			dialog(i).aux = 4  ! left/nonhighlighted
			dialog(i).tpos = 1 ! clickable
		Endif
	Enddo
C
C	check on add/remove buttons
C
	dialog(10).dtype = DLG_TEXT
	dialog(11).dtype = DLG_TEXT
	dialog(11).aux = 2
	dialog(10).aux = 2
	if (nums(side) .gt. 1) then
		dialog(11).dtype = DLG_BUTTON
	Endif
	if (nums(side) .lt. max_rgns) then
		dialog(10).dtype = DLG_BUTTON
	Endif
C
C	Check on number of sub volumes
C
	If (numrgns .lt. 2) Then
		dialog(bv_remove_svol).dtype = DLG_NOP
	Else
		dialog(bv_remove_svol).dtype = DLG_BUTTON
	Endif
	If (numrgns .ge. MAX_SVOLS) Then
		dialog(bv_add_svol).dtype = DLG_NOP
	Else
		dialog(bv_add_svol).dtype = DLG_BUTTON
	Endif
C
C	redraw
C
	Call qenter(REDRAW,0)
C
	return
	End
C
C	routine to read/write the region files
C
	subroutine rgnio(cmd,nums,types,rgns)

	Implicit None

	Include '../libs/vt_inc.f'

	Integer*4	cmd,i,j
	Character*200	rgn
C
	Integer*4	MAX_SVOLS
	Parameter	(MAX_SVOLS = max_rgns)
C
	Character*40	rgns(MAX_SVOLS,max_rgns)
	Integer*4	types(MAX_SVOLS,max_rgns)
	Integer*4	nums(MAX_SVOLS)
C
	Character*40	trgns(max_rgns)
	Integer*4	ttypes(max_rgns)
	Integer*4	tnum
C
	Logical		error
C
C	Save
C
	tnum = numrgns
	Do i=1,numrgns
		trgns(i) = rgnnames(i)
		ttypes(i) = rgntypes(i)
	Enddo
C
C	on cmd read or write
C
	if (cmd .eq. IPC_READ) then
		Do j=1,tnum
			Call roi_to_rgn(trgns(j),rgn)
			Call read_rgns(rgn,error)
			nums(j) = numrgns
			Do I=1,nums(j)
				types(j,i) = rgntypes(i)
				rgns(j,i) = rgnnames(i)
			Enddo
		Enddo
	Else if (cmd .eq. IPC_WRITE) then
		Do j=1,tnum
			numrgns = nums(j)
			Do I=1,nums(j)
				rgntypes(i) = types(j,i)
				rgnnames(i) = rgns(j,i) 
			Enddo
			Call roi_to_rgn(trgns(j),rgn)
			Call write_rgns(rgn,error)
		Enddo
		if (error) Call bv_error(
     +  		"Unable to write a regions file|"//
     +			"Check file permissions or disk space}",0)
	Endif
C
C	Restore
C
	numrgns = tnum
	Do i=1,numrgns
		rgnnames(i) = trgns(i)
		rgntypes(i) = ttypes(i)
	Enddo
C
	Return
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
