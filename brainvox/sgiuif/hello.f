	Program hello

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'

	Integer*4	status
	Integer*4	device
	Integer*4	wid1,wid2
	Integer*2	data
	Record /ditem/	dialog(100)
	
	Call foregr

C	Call minsiz(512,512)	! open 512x512 window
	Call prefpo(100,100+512,100,100+512)
	wid1=winope("hello",5)
	call rgbmod
	call gconfi

C	Call minsiz(512,512)	! open 512x512 window
C	wid2=winope("hello2",6)
C	call rgbmod
C	call doubleb
C	call gconfi

	Call INIT_DLG(status)
	If (status .ne. 0) Call Exit(1)
	Call GET_DLG('hello.res',1,100,dialog,status)
	If (status .ne. 0) Call Exit(1)
C
	dialog(7).aux=Newpup()
	Call addtop(dialog(7).aux,
     +		'Options %t%s|NULL %d|one %r1%I|two  %r1|three %r1%l',
     +		51,0)
	Call addtop(dialog(7).aux,
     +		'toggle %I%l|toggle2 %i',22,0)
C
	Call qdevic(WINQUI)
	Call qenter(REDRAW,0)	! force a redraw
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! end of program
		write(*,*) "WINQUI",data
		Call exit(0)
	Endif
	If (device .eq. INPTCH) then
		If (data .ne. 0) Call winset(data)
		write(*,*) "INPTCH",data
		goto 10
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		write(*,*) "Redraw",data
		data = winget()
		Call winset(wid1)
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) Call Exit(1)
		Call swapbu
C		Call winset(wid2)
C		Call RESHAP	! do not forget !!!
C		Call DRAW_DLG(dialog,status)
C		If (status .ne. 0) Call Exit(1)
C		Call swapbu
		Call winset(data)
		Goto 10
	Endif
	write(*,*) "Before",device,data
	Call HANDLE_DLG(device,data,dialog,status) ! item 2 is the textedit
	If (status .ne. 0) Call Exit(1)
	If (device .eq.  DLG_NOPEVNT) goto 10
        call hand_cursor
	write(*,*) "After",device,data
	If (device .eq.  DLG_BUTEVNT) then	! button 3=ok 4=cancel.
		If (data .eq. 3) then
		write(*,*)'Filename:',dialog(2).text(1:dialog(2).tlength)
		Elseif (data .eq. 4) then
		write(*,*)'No selection.'
		Endif
		Call exit(0)
	Endif
	Goto 10
	End

