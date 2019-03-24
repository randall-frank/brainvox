	Program brainvox_calc

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'calcuif.inc'

	logical		error,quitflag,found
C
	Real*4		volume(max_rgns,max_rgns)
	Real*4		les_vol(max_rgns)
C
	real*4		area,fixedIPS
	Integer*4	counts(127)
	Integer*4	trace(4096,2),num
C
	Integer*4	status,wid,x,y,i,j
	Integer*2	data
	Character*100	tstr,fname
	Record /ditem/	dialog(100)
C
	Logical		done
	Character*200	roi,rgn,dir
C
	Integer*4	slice,side,use_v2
	byte		image(512,512)
C
	integer*4	iargc,strlen

	external 	iargc,strlen
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
1	Format(" Welcome to BRAINVOX_CALC version:",A15,/,
     +	       " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
	Endif
C
	if (iargc() .lt. 1) then
2		write(*,17)
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
C	get the window
C
	Call foregr
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-205)/2
	call prefpo(x,x+700,y,y+205)
	wid=winope("calc",4)     
	Call doubleb
	Call RGBmod
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('calcuif.r',1,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
	Call qdevic(TIMER0)
	Call noise(TIMER0,120)  !every 2 seconds
	Call qdevic(WINQUI)
	Call qenter(REDRAW,wid)	! force a redraw
C
C	New version of all this stuff
C
	use_v2 = 1
	If (use_v2 .eq. 1) Then
		Call calc_volumes(volume,les_vol,dialog)
		goto 999
	Endif
C
	quitflag = .false.
C
	Call doevents(dialog,quitflag)
	if (quitflag) goto 999
C
C	fix the interpixel for the image size
C	ie if images are less than 512x512 then interpixel spacing
C	must be reduced by the ratio
C
	fixedIPS = (interpixel*float(imagesize))/512.0
C
C	for each side
C
C	Do Side = 1,2

	done = .false.
	side = 0
	Do while (.not. done)
C
C	For all sub volumes
C
		side = side + 1
		Call indexnum_to_roi(side,roi,i)
		If (i .eq. -1)  Then
			done = .true.
			goto 500
		Endif
C
C	read the region def files
C
		Call roi_to_rgn(roi,rgn)
		Call read_rgns(rgn,error)
		write(unit=dialog(worktext2).text,fmt=11) roi(1:strlen(roi))
		if (error) goto 500
11	Format("Working on ",A)
C
C	init volume space
C
	Do I=1,numrgns
		Do J=1,numrgns
			volume(I,J) = 0.0
		Enddo
	Enddo
C
	do slice = 1, realimages
		write(unit=dialog(worktext).text,fmt=10) slice,realimages
10	Format("Working on slice ",I3.3," of ",I3.3)
		Call qenter(REDRAW,wid)
		Call doevents(dialog,quitflag)
		if (quitflag) goto 999
C
C	compute a slice worth
C
C	Clear the buffer Image
C
		Do J=1,512 
			Do I=1,512
				image(i,j) = 0
			Enddo
		Enddo
C
C	First for each lesion
C
		Do I=1,numrgns
C
C	read each ROI
C
		   If (rgntypes(i) .ge. 3) then
			error = .false.
			Call name_changer(rgnnames(I),slice,tstr,error)
			if (error) goto 300
			Call roi_to_dir(roi,dir)
			fname = dir(1:strlen(dir))//"/"
     +				//tstr(1:strlen(tstr))
C
C       special case for hemispheres
C
        		If (i .eq. 1) Then
				fname=tstr(1:strlen(tstr))
			Endif
			Call read_roi_file(fname,trace,num,error)
			if ((error).or.(num .lt. 4)) goto 300
			Call fingers_cursor
C
			Call CalcArea(trace,num,area)
C
C	Add area on
C
			volume(I,I)=volume(I,I) +
     +				(area*(fixedIPS**2))*interslice
C
			Call PaintArea(trace,num,I,image)
C
		   Endif
C
300		   Continue
C
		   Call doevents(dialog,quitflag)
		   if (quitflag) goto 999
C
		Enddo
C
C	Now for other ROIs    
C
		Do I=1,numrgns
C
C	read each ROI
C
		   If (rgntypes(i) .le. 2) then
			error = .false.
			Call name_changer(rgnnames(I),slice,tstr,error)
			if (error) goto 200
			Call roi_to_dir(roi,dir)
			fname = dir(1:strlen(dir))//"/"
     +				//tstr(1:strlen(tstr))
C
C       special case for hemispheres
C
        		If (i .eq. 1) Then
				fname=tstr(1:strlen(tstr))
			Endif
			Call read_roi_file(fname,trace,num,error)
			if ((error).or.(num .lt. 4)) goto 200
			Call fingers_cursor
C
			Call CalcArea(trace,num,area)
C
C	Add area on
C
			volume(I,I)=volume(I,I)+
     +				(area*(fixedIPS**2))*interslice
C
			Call IntersectArea(trace,num,counts,image)
C
C	look through counts looking for intersections with lesions
C
			Do J=1,numrgns
				If (counts(j) .ne. -1) then
					volume(i,j) = volume(i,j) +
     +						(float(counts(j))*
     +						(fixedIPS**2))*interslice
					volume(j,i) = volume(i,j)
				Endif
			Enddo
C
		   Endif
200		   Continue
C
		   Call doevents(dialog,quitflag)
		   if (quitflag) goto 999
C
		Enddo
C
C	next slice
C
	Enddo
C
C	output files
C
		Call roi_to_rgn(roi,rgn)
		tstr = rgn(1:strlen(rgn))//".data "
		Call outputdata(volume,tstr)
C
C	next side
C
500	continue
	Enddo
C
C	quit routine  
C
999	continue
C
	Call arrow_cursor
C
C	since we are an upper life form we should send IPC_QUIT
C	and cut the umbilical
C
	Call send_ipc(1,IPC_QUIT)
C
	call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)
C
	End
C
C	routine that needs to be called periodically to handle events
C
	subroutine doevents(dialog,quitflag)
C
	Implicit None
C
	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'calcuif.inc'

	logical		quitflag

	Integer*4	status
	Integer*4	device,size,message(60)
	Integer*2	data
	Record /ditem/	dialog(100)
C
C	Any waiting events??? 
C
10	if (qtest() .eq. 0) return
C
	device=qread(data)	! top of event loop
C	If (device .eq. WINQUI) then	! end of program
C		goto 10     
C	Endif
	if (device .eq. TIMER0) then
		size = 60
		call check_ipc(size,message)
		if (message(1) .eq. IPC_QUIT) quitflag = .true.
		return
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) Then         
			quitflag = .true.
			return
		endif
		Call swapbu
		goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) then
		quitflag = .true.
		return
	endif
	If (device .eq.  DLG_NOPEVNT) goto 10 
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		if (data .eq. quitbutton) then
			quitflag = .true.
			return
		Endif
	Endif
C
	goto 10
C
	end
