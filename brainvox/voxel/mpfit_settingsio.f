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
C    MODULE NAME    : Load/Save MP_FIT settings
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 12 Jan 93       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to handle the dialog MP_FIT settings I/O
C				dialog
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
	Subroutine mp_fit_settings_io(data_id)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	
	Integer*4	data_id,status
	Record /ditem/  dialog(100)
C
C       IPC defines
C
        Integer*4       size,message(60)
C
	Character*256	diskfiles(100)
	Character*256	tstr,tempstr
	Integer*4	cur_set
	Integer*4	device,i,j,wid,st
	Integer*2	data
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
	shm_data = data_id
        shmem_owner = 0
        Call vt_init_shmem
C
C       get the window
C
        call prefsi(300,130)
        wid=winope("Settings files",14)
        Call doubleb
        Call RGBmod
        Call gconfi
        Call qdevic(WINQUI)
        Call INIT_DLG(status)
        If (status .ne. 0) Call bv_exit(1)
        Call GET_DLG_COMP('voxeluif.r',3,100,dialog,status)
        If (status .ne. 0) Call bv_exit(1)
C
C       Menus for the pointsets
C
	dialog(mpio_menu).aux = 0
	Call mpio_scan_disk(dialog(mpio_menu).aux,". ",diskfiles)
C
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,wid)
C
C       redraw comes from setup_dialog
C
10      device=qread(data)      ! top of event loop
C
C       timer stuff
C
        if (device .eq. TIMER0) then
                size = 60
                call check_ipc(size,message)
                if (message(1) .eq. IPC_QUIT) goto 999
                goto 10
        Endif
C
C       handle redraw
C
        If (device .eq. REDRAW) then    ! redraw screen
                Call RESHAP     ! do not forget !!!
                Call ortho2(0.0,300.0,0.0,130.0)
                Call DRAW_DLG(dialog,status)
                If (status .ne. 0) then
                         goto 999
                Endif
		Call swapbu
	Endif
        Call HANDLE_DLG(device,data,dialog,status)
        If (status .ne. 0) goto 999
        If (device .eq.  DLG_NOPEVNT) goto 10
C
C       handle buttons
C
        If (device .eq.  DLG_BUTEVNT) then
		j = dialog(mpio_edit).tlength
		tstr = dialog(mpio_edit).text(1:j)

		If (data .eq. mpio_load) then
			Call send_message(IPC_MPIOLOAD,tstr)
		Else if (data .eq. mpio_save) then
			Call send_message(IPC_MPIOSAVE,tstr)
			Call mpio_scan_disk(dialog(mpio_menu).aux,
     +				". ",diskfiles)
		Else if (data .eq. mpio_done) then
			goto 999
		Endif

		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C	handle edit box
C
	If (device .eq.  DLG_EDTEVNT) then
		If (data .eq. mpio_edit) then
			j = dialog(mpio_edit).tlength
			tstr = dialog(mpio_edit).text(1:j)
		Endif
		goto 10
	Endif
C
C       handle menu events
C
        If (device .eq. DLG_POPEVNT) then
		If (data .eq. mpio_menu) Then
			i = dialog(data).tpos
			If (i .eq. 100) Then
				Call mpio_scan_disk(dialog(mpio_menu).aux,
     +					". ",diskfiles)
			Else
				dialog(mpio_edit).text = diskfiles(i)
				dialog(mpio_edit).tlength = 
     +					index(diskfiles(i)," ")-1
				dialog(mpio_edit).tpos =
     +					dialog(mpio_edit).tlength
			Endif
			Call qenter(REDRAW,wid)
		Endif
		goto 10
	Endif
C
C	get next event
C
	goto 10
C
C	Exit 
C
999	Call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)

	Return
	End
C
C	Send The IPC message
C
	Subroutine	send_message(cmd,file)

	Implicit None

	Include '../libs/vt_inc.f'

	Integer*4	cmd,file(64)  ! 256 bytes

	Integer*4	msg(65),I

	msg(1) = cmd
	Do I=1,64
		msg(i+1) = file(i)
	Enddo
	Call send_ipc(65,msg)

	Return
	End
C
C	Save the current settings
C
	Subroutine	MP_FIT_load_settings(file,rots)

	Implicit None

	Include 'rotsdef.inc'

	Record  /rotsrec/       rots
	Character*256		file,tfile,junk

	Integer*4	i,j
	Logical		found

	tfile = file
	i = index(tfile," ")-1
	If (index(file,".MPSET") .eq. 0) Then
		i = index(file," ")
		If (i .eq. 0) Return

		i = i - 1
		tfile = file(1:i)//".MPSET "
		i = i + 6
	Endif

	Inquire(file=tfile(1:i),exist=found)

	Open(unit=66,file=tfile(1:i),status="old",
     +		form="formatted",err=999)

	Read(66,5,err=997,end=997) junk
	Read(66,*,err=997,end=997) rots.sthickness
	Read(66,*,err=997,end=997) rots.ipixel
	Read(66,*,err=997,end=997) rots.slew(1),rots.slew(2)
	Read(66,*,err=997,end=997) rots.tilt(1),rots.tilt(2)
        Read(66,*,err=997,end=997) rots.uptrans(1),rots.uptrans(2),
     +		rots.uptrans(3)
	Read(66,*,err=997,end=997) rots.x_off
	Read(66,*,err=997,end=997) rots.n_slices(1)
	Read(66,5,err=997,end=997) junk
	rots.outname = junk(1:200)
5	Format(A200)

	rots.xshift = 0
	Read(66,*,err=995,end=995) rots.xshift

995	Close(66)

	write(*, 50)tfile(1:i)
50	Format("Loaded MP_FIT settings from ",A)

	Return

997	write(*, 996)tfile(1:i)
996	Format("Unable to read the file ",A,".")
	Close(66)

	Return

999	write(*, 998)tfile(1:i)
998	Format("Unable to open the file ",A," for reading.")

	Return
	End
C
C	Save the current settings
C
	Subroutine	MP_FIT_save_settings(file,rots)

	Implicit None

	Include 'rotsdef.inc'

	Record  /rotsrec/       rots
	Character*256		file,tfile

	Integer*4	i
	Logical		found

	tfile = file
	i = index(tfile," ")-1
	If (index(file,".MPSET") .eq. 0) Then
		i = index(file," ")
		If (i .eq. 0) Return

		i = i - 1
		tfile = file(1:i)//".MPSET "
		i = i + 6
	Endif

	Inquire(file=tfile(1:i),exist=found)

	If (found) Then
		Open(unit=66,file=tfile(1:i),status="old",
     +			form="formatted",err=999)
	Else
		Open(unit=66,file=tfile(1:i),status="new",
     +			form="formatted",err=999)
	Endif
	write(66,1,err=997) "1.0 MP_FIT settings"
1	Format(A)

10	Format(F10.5,F10.5,F10.5)
11	Format(A200)
12	Format(I8)
	write(66,10,err=997) rots.sthickness
        write(66,10,err=997) rots.ipixel
        write(66,10,err=997) rots.slew(1),rots.slew(2)
        write(66,10,err=997) rots.tilt(1),rots.tilt(2)
        write(66,10,err=997) rots.uptrans(1),rots.uptrans(2),
     +		rots.uptrans(3)
        write(66,10,err=997) rots.x_off
        write(66,12,err=997) rots.n_slices(1)
        write(66,11,err=997) rots.outname

	write(66,10,err=997) rots.xshift

	Close(66,err=995)

	write(*, 50)tfile(1:i)
50	Format("Saved MP_FIT settings to ",A)

	Return

997	Close(66,err=995)
995	write(*, 996)tfile(1:i)
996	Format("Unable to write the file ",A,".")

	Return

999	write(*, 998)tfile(1:i)
998	Format("Unable to open the file ",A," for writing.")

	Return
	End
C
C	Routine to scan for valid MP_FIT files
C
	Subroutine mpio_scan_disk(menu,dir,files)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'

	Integer*4	menu
	Character*256	dir
	Character*256	files(100)

	Integer*4	i,dfp,j
	Character*256	tstr,tmp

	If (menu .ne. 0) Call freepu(menu)
C
C	start the menu
C
	menu = newpup()
	tstr = "Scan disk for files %x100%l}"
        Call addtop(menu,tstr,index(tstr,"}")-1,0)
C
C	open the DIR
C
	tstr = dir
	i = index(tstr," ")
	tstr(i:i) = char(0)
	Call dir_open(dfp,tstr)
	If (dfp .eq. 0) Return
C
C	read the filenames in the CWD
C
	j = 1
	Do while ((tstr(1:1) .ne. char(0)) .and. (j .lt. 99))
		Do I=1,256
			tstr(i:i) = " "
		Enddo
		Call dir_read(dfp,tstr)
		If (tstr(1:1) .ne. char(0)) Then
			i = index(tstr,".MPSET")
			If ((i .ne. 0).and.
     +				(tstr(i+6:i+6) .eq. char(0))) Then
				Write(tmp,10) tstr(1:i-1),j
10	Format(A,"%x",I2.2,"}")
				files(j) = tstr(1:i-1)
				j = j + 1
        			Call addtop(menu,tmp,
     +					index(tmp,"}")-1,0)
			Endif
		Endif
	Enddo
C
C	close up shop
C
	Call dir_close(dfp)
C
	Return
	End
