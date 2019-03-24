	Program Brainvox_prefs

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'vtuif.inc'
	Include 'prefs.f'

	logical		error

	Integer*4	status,i
	Integer*4	device,err
	Integer*4	wid,x,y
	Integer*2	data
	character*120	tstr
	Record /ditem/	dialog(100)
C
	Integer*4	irisglmouse,noreaper,hwinterp
	Real*4		backuprate
C
	integer*4	iargc,bv_fork,slength,strlen
	external 	iargc,bv_fork,slength,strlen
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)

C       print version and program ID
C
        If ((iargc() .lt. 1).or.(bv_test_option(OPTS_TESTING))) Then
        write(*,1)version,com_date
1       Format(" Welcome to BRAINVOX_PREFS version:",A15,/,
     +         " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
        Endif
C
C       Preferences definitions
C
	Call get_pref_i(PREF_NOREAPER,noreaper)
	Call get_pref_i(PREF_IRISGLMOUSE,irisglmouse)
	Call get_pref_i(PREF_3DHWINTERP,hwinterp)
	Call get_pref_f(PREF_BACKUPRATE,backuprate)
C
C       Get the window
C
	x = (getgde(GDXPMA)-700)/2
	y = (getgde(GDYPMA)-235)/2
	call prefpo(x,x+700,y,y+235)
	wid=winope("Brainvox preferences",20)
	call rgbmod
	call doubleb
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	Call GET_DLG_COMP('vtuif.r',4,100,dialog,status)
	If (status .ne. 0) Call bv_exit(1)
C
	dialog(bv_mouse_check).aux = irisglmouse
	dialog(bv_noreap_check).aux = noreaper
	dialog(bv_hwinterp_check).aux = hwinterp
	write(dialog(bv_backup_edit).text,599) backuprate
	dialog(bv_backup_edit).tlength = 8
	dialog(bv_backup_edit).tpos = 8
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
	Endif
C
C	handle buttons
C
	If (device .eq.  DLG_CHKEVNT) then
		If (data .eq. bv_mouse_check) then
			irisglmouse = dialog(bv_mouse_check).aux
		Else if (data .eq. bv_noreap_check) then
			noreaper = dialog(bv_noreap_check).aux
		Else if (data .eq. bv_hwinterp_check) then
			hwinterp = dialog(bv_hwinterp_check).aux
		Endif
	Endif
C
C	handle buttons
C
	If (device .eq.  DLG_BUTEVNT) then	
		If (data .eq. bv_prefs_close) then   !quit
			goto 999
		Else if (data .eq. bv_prefs_save) then !edit info
			Call set_pref_i(PREF_NOREAPER,noreaper)
			Call set_pref_i(PREF_IRISGLMOUSE,irisglmouse)
			Call set_pref_f(PREF_BACKUPRATE,backuprate)
			Call set_pref_i(PREF_3DHWINTERP,hwinterp)
			Call file_pref(1)
			goto 999
		Endif
	Endif
C
C	handle edit text
C
	if (device .eq. DLG_EDTEVNT) then
		i = dialog(data).tlength
		If (data .eq. bv_backup_edit) then  
			read(dialog(data).text(1:i),*,
     +				err=600,end=600) backuprate
600			write(dialog(data).text,599) backuprate
599			Format(f8.2)
			dialog(data).tlength = 8
			dialog(data).tpos = 8
			Call qenter(REDRAW,wid)
		Endif
	Endif
C
	Goto 10
C
C	quit routine  (no saving )
C
999	continue
	call winclo(wid)
	Call bv_exit(0)

	End
