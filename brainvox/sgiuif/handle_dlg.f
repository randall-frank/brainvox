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
C    MODULE NAME    : handle_dlg
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : sgiuif
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 08 Aug 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine converts the system event into
C			a SGIUIF event if possible
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
	Subroutine handle_dlg(dev,data,dialog,status)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	integer*4	fmwid
	external	fmwid

	Integer*4	dev,len
	Integer*2	data
	Integer*4	status
	Record	/ditem/	dialog(*)

	Integer*4	I,editindex,mx,my,buffers,j,k
	Integer*4	rects(4,5)  !temp sbar rects... five rects(4i*4)
	Character*200	text
C
	Integer*4	max,min,lin,pag  !scroll bars motions...
	Integer*4	temp,fsave
	Integer*4	ffff
	parameter	(ffff = 65535)
	Logical		dlg_inrect
	Logical		dlg_winrect
C
	external	dlg_inrect
	external	dlg_winrect
C
C	Set screen status... Frontbuffer,zbuffer false...
C
	buffers= getbuf()
	Call frontb(.true.)
	Call backbu(.false.)
	Call zbuffe(.false.)
C
	Status=0
C
C	copy the rectangle into the global...
C
	Do I=1,4
		dlgrect(I)=dialog(1).rect(I)
	Enddo
C
C	I is the pointer to the current item
C
	I=1
	If (dialog(1).dtype .ne. DLG_HEADER) then
		status=-1
		Return
	Endif
	editindex=dialog(1).tpos
C
C	find the wouse cursor...
C
	Call dlg_g_mouse(mx,my)
C
C	loop through all the items
C
	Do while (dialog(I).dtype .ne. DLG_END)
C
C	next item
C
		I=I+1
C
C	begin a large case of statement for all the item types except header.
C	Note that DLG_HEADER is 1 ... DLG_RADIO is 13
C
		GOTO (100,200,300,400,500,600,700,800,900,1000,1100,
     +			1200,1213),dialog(I).dtype
C
C	this is a user error...out of range (1-12) or DLG_HEADER
C
100		Status=-1
		Return
C
C	DLG_COLOR
C
200		Continue
		Goto 1300
C
C	DLG_END  
C
300		Continue
		Goto 1300
C
C	DLG_LINE 
C
400		Continue
		Goto 1300
C
C	DLG_BOX
C
500		Continue
		Goto 1300
C
C	DLG_TEXT 
C
600		Continue
C
C	clickable and the event is mousedown
C
		If ((dialog(I).tpos .eq. 1).and.(dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect(1),mx,my)) then
C	
C	no feedback...
C
C
C	wait for mouse up in rect
C
				If (dlg_winrect(dialog(I).rect(1))) then
					data=I
					dev=DLG_TXTEVNT
					status=0
					goto 2000
				Else
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Endif
			Endif
		Endif
		Goto 1300
C
C	DLG_EDIT 
C
700		Continue
C
C	if there is no active text edit field this becomes it!!
C
		If (dialog(1).tpos .eq. 0) then
			dialog(1).tpos= I
			dialog(I).tpos= dialog(I).tlength
		Endif
C
C	mouse down...
C
		If ((dev .eq. guibtn()).and.(data .eq. 1).and. 
     +			(dlg_inrect(dialog(I).rect(1),mx,my))) then
			If (I .eq. dialog(1).tpos) Then
C
C	set correct font...
C
				fsave=cur_font
				cur_font=2
				temp=dialog(i).aux
				if (iand(temp,255*256) .ne. 0) then
					cur_font=iand(temp,255*256)/256
					if (cur_font .gt. 10) cur_font=2
				Endif
C
C	click in active edit field
C
				Call dlg_g_mouse(mx,my)
				text=dialog(I).text
				status=mx-dialog(I).rect(1)-4  !point in rect
C
C	find char position which is to the left of the click point
C
				dialog(I).tpos=0
				Do J=1,dialog(I).tlength
					If (jifix(float(fmwid(text,J))
     +						*cortopix)
     +						 .lt. status) then
						dialog(I).tpos=J
					Endif
				Enddo
				cur_font=fsave
				Call dlg_d_edit(dialog(I),1)
				dev=DLG_NOPEVNT
				status=0
				Goto 2000
			Else
C
C	new current edit field
C
				dialog(I).tpos=dialog(I).tlength
				Call dlg_d_edit(dialog(I),1)
C
C	clean up old one...
C
				If (dialog(1).tpos .ne. 0) then
				      Call dlg_d_edit(
     +					dialog(dialog(1).tpos),0)
				Endif
C
C	make event
C
				dev=DLG_EDTEVNT
				data=dialog(1).tpos
C
C	make it global
C
				dialog(1).tpos=I
				status=0
				goto 2000
			Endif
		ElseIf ((Dev .eq. KEYBD).and.
     +			(I .eq. dialog(1).tpos)) then
C
C	keypress in the active window...
C
			status=dialog(I).tpos
			max=dialog(I).tlength
			text=dialog(I).text
C
			If (data .eq. 22) then
C 
C Ctrl-V (paste)
C
				call clip2text(text,len)
			if (len .gt. 0) then
C
C	set correct font...
C
				fsave=cur_font
				cur_font=2
				temp=dialog(i).aux
				if (iand(temp,255*256) .ne. 0) then
					cur_font=iand(temp,256*255)/256
					if (cur_font .gt. 10) cur_font=2
				Endif
C
C	if the new text fits into the rect then make it so...
C
				Do while(fmwid(text,len) .gt.
     +			dialog(I).rect(3)-dialog(I).rect(1)-5)
					len = len - 1 
				Enddo
				dialog(I).text=text(1:len)
				dialog(i).tpos=len
				dialog(I).tlength=len
				cur_font=fsave
				Call dlg_d_edit(dialog(I),1)
			endif

			dev=DLG_NOPEVNT
			Status=0
			goto 2000
C
			else if (data .eq. 3) then
C
C Ctrl-C (copy)
C
				call text2clip(text(1:max))
				dev=DLG_NOPEVNT
				Status=0
				goto 2000
C
			else if ((data .ge. 32).and.
     +				(char(data) .le. '~')) then
C
C	set correct font...
C
				fsave=cur_font
				cur_font=2
				temp=dialog(i).aux
				if (iand(temp,255*256) .ne. 0) then
					cur_font=iand(temp,256*255)/256
					if (cur_font .gt. 10) cur_font=2
				Endif
C
C	insert a character  ' ' - '~'
C
				If (max .lt. 200) then
C
C	attempt the insert
C
					Do J=max,status+1,-1 
						text(J+1:J+1)=text(J:J)
					Enddo
				Text(status+1:status+1)=char(data)
					max=max+1
					status=status+1
C
C	if the new text fits into the rect then make it so...
C
					If (fmwid(text,max) .lt.
     +			dialog(I).rect(3)-dialog(I).rect(1)-5) then
						dialog(I).text=text
						dialog(i).tpos=status
						dialog(I).tlength=max
					Endif
				Endif
				cur_font=fsave
				Call dlg_d_edit(dialog(I),1)
				dev=DLG_NOPEVNT
				Status=0
				goto 2000
			Elseif (data .eq. 8) then
C
C	handle the backspace key...
C
				If (status .ge. 1) then
					Do J=status+1,max
						text(J-1:J-1)=text(J:J)
					Enddo
					max=max-1
					status=status-1
					dialog(I).text=text
					dialog(i).tpos=status
					dialog(I).tlength=max
				Endif
				Call dlg_d_edit(dialog(I),1)
				dev=DLG_NOPEVNT
				Status=0
				goto 2000
			Endif
C
C	check for arrow keys
C
		Elseif ((dev .eq. RIGHTA).and.(data .eq. 1).and.
     +				(I.eq.dialog(1).tpos)) Then
C
C	move right
C
			status=dialog(I).tpos
			status=status+1
			If (status .le. dialog(I).tlength) Then
				dialog(I).tpos=status
			Endif
			Call dlg_d_edit(dialog(I),1)
			dev=DLG_NOPEVNT
			Status=0
			goto 2000
		Elseif ((dev .eq. LEFTAR).and.(data .eq. 1).and.
     +			(I.eq.dialog(1).tpos)) Then
C
C	move left 
C
			status=dialog(I).tpos
			status=status-1
			If (status .ge. 0) Then
				dialog(I).tpos=status
			Endif
			Call dlg_d_edit(dialog(I),1)
			dev=DLG_NOPEVNT
			Status=0
			goto 2000
		Elseif ((dev .eq. DELKEY).and.(data .eq. 1).and.
     +			(I.eq.dialog(1).tpos)) Then
C
C	delete key 
C
			dialog(I).tlength=0
			dialog(I).tpos=0
			dialog(I).text=' '
			Call dlg_d_edit(dialog(I),1)
			dev=DLG_NOPEVNT
			Status=0
			goto 2000
C
		Elseif (((dev .eq. RETKEY).or.(dev .eq. TABKEY))
     +	.and.(I.eq. dialog(1).tpos).and.(data .eq. 1)) Then
C
C	tab or return key  Close up old dialog...
C
			Call dlg_d_edit(dialog(I),0)
C
C	find the next edit field in the list...
C
			J=I+1
705			If (dialog(J).dtype .eq. DLG_END) goto 710
			If (dialog(J).dtype .eq. DLG_EDIT) goto 720
			J=J+1
			goto 705
C
C	start over from the beginning
C
710			J=1
715			If (dialog(J).dtype .eq. DLG_END) Then
C
C	this should never occur
C
				J=I
				goto 720
			Endif
			If (dialog(J).dtype .eq. DLG_EDIT) goto 720
			J=J+1
			goto 715
C
C	current edit becomes J,  I is the one we just left...
C
720			dialog(1).tpos=J
			dialog(J).tpos=dialog(J).tlength
			Call dlg_d_edit(dialog(J),1)
			dev=DLG_EDTEVNT
			data=I
			Status=0
			goto 2000
		Endif
		Goto 1300
C
C	DLG_SBAR 
C
800		Continue
C
C	the event is mousedown
C
		If ((dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect(1),mx,my)) then
C
C	find the five rects...
C
				Call dlg_frects(dialog(I),rects)
C
C	feedback
C			
				K=0
				Do J=1,5
				If (dlg_inrect(rects(1,J),mx,my)) then
					K=J
     					Call dlg_d_sbar(dialog(I),K)
				Endif
				Enddo
C
C	handle thumb special...
C
				If (K .eq. 5) then
C
C	scroll W/mouse...
C
				    Call dlg_dosbar(dialog(I),mx,my)
				    Call dlg_d_sbar(dialog(I),0)
					data=I
					dev=DLG_BAREVNT
					status=0
					goto 2000
C
				ElseIf (dlg_winrect(rects(1,k))) then
C
C	wait for mouse up in rect
C
C	find the min,max and page,line values
C
					status=dialog(I).tlength
					min=status/65536
					max=iand(status,ffff)
					Status=dialog(I).tpos
					lin=status/65536
					pag=iand(status,ffff)
C
C	move 
C
					If (K .eq. 1) then
					dialog(I).aux=dialog(I).aux-lin
					ElseIf (K .eq. 2) then
					dialog(I).aux=dialog(I).aux+lin
					ElseIf (K .eq. 3) then
					dialog(I).aux=dialog(I).aux-pag
					ElseIf (K .eq. 4) then
					dialog(I).aux=dialog(I).aux+pag
					Endif
					If (dialog(I).aux .lt. min)
     +						dialog(I).aux = min
					If (dialog(I).aux .gt. max)
     +						dialog(I).aux = max
C
C	change state
C
					Call dlg_d_sbar(dialog(I),0)
					data=I
					dev=DLG_BAREVNT
					status=0
					goto 2000
				Else 
					Call dlg_d_sbar(dialog(I),0)
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Endif
C
			Endif
		Endif
		Goto 1300
C
C	DLG_MENU 
C
900		Continue
C
C	the event is mousedown
C
		If ((dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect(1),mx,my)) then
C	
C	feedback...
C
				Call dlg_d_menu(dialog(I),1)
C
C	do the popup...
C
				If (dialog(I).aux .ne. 0) then
				     Call gflush
				     dialog(I).tpos=dopup2(dialog(I).aux)
				Else
				     dialog(I).tpos=-1
				Endif
				Call dlg_d_menu(dialog(I),0)
				If (dialog(I).tpos .eq. -1) then
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Else
					dev=DLG_POPEVNT
					data=I
					Status=0
					goto 2000
				Endif
			Endif
		Endif
		Goto 1300
C
C	DLG_CHECK 
C
1000		Continue
C
C	the event is mousedown
C
		If ((dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect(1),mx,my)) then
C	
C	feedback...
C
				Call dlg_d_checkbox(dialog(I),1)
C
C	wait for mouse up in rect
C
				If (dlg_winrect(dialog(I).rect)) then
C
C	change state
C
					dialog(I).aux=1-dialog(I).aux
					Call dlg_d_checkbox(dialog(I),0)
					data=I
					dev=DLG_CHKEVNT
					status=0
					goto 2000
				Else 
					Call dlg_d_checkbox(dialog(I),0)
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Endif
			Endif
		Endif
		Goto 1300
C
C	DLG_RADIO 
C
1213		Continue
C
C	the event is mousedown
C
		If ((dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect(1),mx,my)) then
C	
C	feedback...
C
				Call dlg_d_radio(dialog(I),1)
C
C	wait for mouse up in rect
C
				If (dlg_winrect(dialog(I).rect)) then
C
C	change state to 1 for that one selected...
C	and make sure it is the only one contigously in the list...
C
					If (dialog(I).aux .eq. 0) then
     						dialog(I).aux=1
					Endif
					Call dlg_d_radio(dialog(I),0)
C
C	cancel out those before...(note: the first must be DLG_HEADER)
C
				j=I-1
1234				If (dialog(J).dtype 
     +						.eq. DLG_RADIO) then
					dialog(J).aux=0
					Call dlg_d_radio(dialog(j),0)
					j=j-1
					goto 1234
				Endif
C
C	and those following...(note: the last must be DLG_END)
C
					j=I+1
1235				If (dialog(J).dtype .eq. DLG_RADIO)
     +						then
					dialog(J).aux=0
					Call dlg_d_radio(dialog(j),0)
					j=j+1
					goto 1235
				Endif
C
					data=I
					dev=DLG_RADEVNT
					status=0
					goto 2000
				Else 
					Call dlg_d_radio(dialog(I),0)
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Endif
			Endif
		Endif
		Goto 1300
C
C	DLG_BUTTON
C
1100		Continue
C
C	the event is mousedown
C
		If ((dev .eq. guibtn()).and.
     +			(data .eq. 1)) then
C
C	in rect to begin with???
C
			If (dlg_inrect(dialog(I).rect,mx,my)) then
C	
C	feedback...
C
				Call dlg_d_button(dialog(I),1)
C
C	wait for mouse up in rect
C
				If (dlg_winrect(dialog(I).rect)) then
C
C	change state
C
					Call dlg_d_button(dialog(I),0)
					data=I
					dev=DLG_BUTEVNT
					status=0
					goto 2000
				Else 
					Call dlg_d_button(dialog(I),0)
					dev=DLG_NOPEVNT
					status=0
					goto 2000
				Endif
			Endif
		Endif
		Goto 1300
C
C	DLG_NOP   
C
1200		Continue
		Goto 1300
C
C	the item has been handled loop back for the next
C
1300		Continue 
	Enddo
C
	status=0
C
2000	Continue
C	
C	restore screen/buffer status and return
C
	If (buffers .eq. 1) Then
		Call backbu(.true.)
		Call frontb(.false.)
	ElseIf (buffers .eq. 2) then
		Call backbu(.false.)
		Call frontb(.true.)
	Elseif (buffers .eq. 3) then
		Call backbu(.true.)
		Call frontb(.true.)
	Endif
	If (zbuffer) Call zbuffe(.true.)
C	
	Return
	End
C
C	subroutines local to this file...
C
C
C	this routine will locate the mouse cursor in the viewport coordinate
C	system...
C
	Subroutine	dlg_g_mouse(mx,my)

	Implicit None
	
	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.f'

	Integer*4	mx,my
	Integer*2	l,r,b,t
	Integer*4	left,right,bot,top,wx,wy

C
C	get the viewport
C
	Call getvie(l,r,b,t)
C
	left=l
	right=r
	bot=b
	top=t
C
C	get the cursor position
C
	mx=getval(CURSRX)
	my=getval(CURSRY)
C
C	get window position
C
	Call getori(wx,wy)
C
C	now make the mx,my in the viewport coords
C
	mx=mx-(wx+left)
	my=my-(wy+bot)
C
C	assume the viewport has been mapped into the dialog rect space
C	as defined by dlgrect(1-4) left top right botttom
C
C	so:  mx-left      X-r(1)
C           ---------- = ---------
C	    right-left   r(3)-r(1)
C
	mx=jifix(((float(mx-left)/float(right-left))*
     +		float(dlgrect(3)-dlgrect(1)))+dlgrect(1))
	my=jifix(((float(my-top)/float(bot-top))*
     +		float(dlgrect(4)-dlgrect(2)))+dlgrect(2))
C
	Return
	End
C
C	this routine will poll the mouse waiting for it to release
C	If the mouse is still in the rectangle on release it will return 
C	true otherwise false...
C
	Logical	function dlg_winrect(rect)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'

	Integer*4	mx,my
	Integer*4	rect(4)
	Logical		dlg_inrect
C
	external	dlg_inrect
C
	Call gfflush
C
C	while button down wait...
C
	Do while (getbut(guibtn()))
C
C	wait
C
	Enddo
C
C	on mouseup is it in the rect???
C
	Call dlg_g_mouse(mx,my)
	dlg_winrect=dlg_inrect(rect,mx,my)
C
	Return
	End  
