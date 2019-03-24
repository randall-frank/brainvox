	Program hello

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'

	Integer*4	status
	Integer*4	device
	Integer*4	wid
	Integer*2	data
	Record /ditem/	dialog(100)
	
	Call prefsi(512,512)	! open 512x512 window
	wid=winope("hello",5)
	Call INIT_DLG(status)
	If (status .ne. 0) Call Exit(1)
	Call GET_DLG_COMP('hello.res',1,100,dialog,status)
	If (status .ne. 0) Call Exit(1)
C
	dialog(7).aux=Newpup()
	Call addtop(dialog(7).aux,'Options %t|one|two|three',24)
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,0)	! force a redraw
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! end of program
		Call exit(0)
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) Call Exit(1)
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) ! item 2 is the textedit
	If (status .ne. 0) Call Exit(1)
	If (device .eq.  DLG_NOPEVNT) goto 10
	If (device .eq.  DLG_BUTEVNT) then	! button 3=ok 4=cancel.
		If (data .eq. 3) then
		Type *,'Filename:',dialog(2).text(1:dialog(2).tlength)
		Elseif (data .eq. 4) then
		Type *,'No selection.'
		Endif
		Call exit(0)
	Endif
	Goto 10
	End
