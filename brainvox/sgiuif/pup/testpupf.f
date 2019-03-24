	Program testpup

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'

	Integer*4	wid,dev,I,col,m,m1,m2
	Integer*2	data
	Character*256	text

	call foregr
	call prefpo(0, 511, 0, 511)
	wid = winope("testpup",7)
	col = 0
	call color(BLACK)
	call clear

	m = newpup()
	m1 = newpup()
	m2 = newpup()

	text = "submenu %t%s|item 0%i|item 1%i|item 2 %i%l|item 3%i}"
	call addtop(m,text,index(text,"}")-1,0)

	text = "main %t%s|goofball|zork %m|gerk %x11|blark %i%x33}"
	call addtop(m1,text,index(text,"}")-1,m)

	text = "rmenu %t%s|radio 1%r0|radio 2%r0%d|radio 3%r0%l%x119|"//
     +		"radio 4%r1|radio 5%r1|radio 6%r1%x55}"
	call addtop(m2,text,index(text,"}")-1,0)

	i = m

	call qdevic(MOUSE1)
	call qdevic(MOUSE2)
	call qdevic(MOUSE3)

	Do while (.true.)
		dev = qread(data)
		If (dev .eq. REDRAW) then
			call reshap
			call clear
		Else if ((dev .eq. MOUSE1).and.(data .eq. 1)) then
			Type *,dopup(i)
		Else if ((dev .eq. MOUSE2).and.(data .eq. 1)) then
			If (i .eq. m) then
				i = m1
			Else if (i .eq. m1) then
				i = m2
			Else
				i = m
			Endif
		Else if ((dev .eq. MOUSE3).and.(data .eq. 1)) then
			col = 1-col
			If (col .eq. 1) then
				call color(WHITE)
			Else
				call color(BLACK)	
			Endif
			call clear
			if (col .eq. 1) Then
				text = "item 1 %I}"
			Else
				text = "item 1 %i}"
			Endif
			Call change(m,2,text,index(text,"}")-1,0)
		Endif
	Enddo

	End
