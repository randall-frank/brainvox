C
C	routine to fill manage dialogs
C
	subroutine setup_dialog(rots,dialog,wid,myset,who)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	Include 'windows.inc'

	Include 'rotsdef.inc'

	Record	/rotsrec/	rots
C
	Integer*4	i
	Integer*4	myset(*),wid,who
	Character*200	tstr1,tstr2,tstr3,tstr
C
	Integer*4	xr,yr,zr
	Real*4		frots(4)
C
	Record /ditem/	dialog(*)
C
C       Setup the Mode:xxx dialog, the MP_FIT slice #, and the Mode:popup
C
	dialog(statemenu).tlength = 8
	tstr1 = "Slices %i%r4%x0}"
	tstr2 = "Arbitrary %i%r4%x1}"
	tstr3 = "MP_FIT %i%l%r4%x2}"
	If (wind2d(curwin).mode .eq. 0) Then
		dialog(statemenu).text = "Mode:Sli"
		tstr1 = "Slices %I%r4%x0}"
	Else if (wind2d(curwin).mode .eq. 1) Then
		dialog(statemenu).text = "Mode:Arb"
		tstr2 = "Arbitrary %I%r4%x1}"
	Else
		dialog(statemenu).text = "Mode:MPF"
		tstr3 = "MP_FIT %I%l%r4%x2}"
	Endif
	Call change(dialog(statemenu).aux,1,tstr1,index(tstr1,"}")-1,0)
	Call change(dialog(statemenu).aux,2,tstr2,index(tstr2,"}")-1,0)
	Call change(dialog(statemenu).aux,3,tstr3,index(tstr3,"}")-1,0)
C
	If (wind2d(curwin).mode .eq. 2) Then
		Do i=0,14
			If (i .eq. wind2d(curwin).n_slice) Then
				write(tstr,66) i+1,i+100
			Else 
				write(tstr,67) i+1,i+100
			Endif
66      Format("Slice ",I2.2," %I%r5%x",I3.3,"}")
67      Format("Slice ",I2.2," %i%r5%x",I3.3,"}")
			Call change(slice_menu,i+1,tstr,index(tstr,"}")-1,0)
		Enddo
	Endif
C
C	setup the text r.=s           
C
	write(unit=dialog(3).text,fmt=5) rots.rots(1)
5	Format('Rx=',I3.3)
	dialog(3).tlength = 6
	write(unit=dialog(4).text,fmt=6) rots.rots(2)
6	Format('Ry=',I3.3)
	dialog(4).tlength = 6
	write(unit=dialog(5).text,fmt=7) rots.rots(3)
7	Format('Rz=',I3.3)
	dialog(5).tlength = 6
C
	write(unit=dialog(cuttext).text,fmt=8) wind2d(curwin).rots(1)
8	Format('Rx=',I3)
	dialog(cuttext).tlength = 6
	write(unit=dialog(cuttext+1).text,fmt=9) wind2d(curwin).rots(2)
9	Format('Ry=',I3)
	dialog(cuttext+1).tlength = 6
	write(unit=dialog(cuttext+2).text,fmt=10) wind2d(curwin).rots(3)
10	Format('Rz=',I3)
	dialog(cuttext+2).tlength = 6
	write(unit=dialog(cuttext+3).text,fmt=11) wind2d(curwin).rots(4)-256
11	Format('D=',I4)
	dialog(cuttext+3).tlength = 6
C
C	scroll bars
C
	Do I=1,3
		dialog(i+5).aux = rots.rots(i)
	Enddo
	Do I=1,4
		dialog(cutbars-1+i).aux = wind2d(curwin).rots(i)
	Enddo
C
C	and the image scroll bar
C
	dialog(15).tlength = 65536 + realimages
	dialog(15).aux = wind2d(curwin).inum2d
	write(unit=dialog(16).text,fmt=12) wind2d(curwin).inum2d
12	Format('Im=',I3.3)
	dialog(16).tlength = 6
C
C	assign the rotations
C
	xr = rots.rots(1)*10
	yr = rots.rots(2)*10
	zr = rots.rots(3)*10
	Call vl_rots(%val(xr),%val(yr),%val(zr),myset)
C
C	Set the dialog to the slices state
C
	dialog(thickedit).dtype = DLG_NOP
	dialog(ipixeledit).dtype = DLG_NOP
	dialog(countedit).dtype = DLG_NOP
	dialog(offedit).dtype = DLG_NOP
	dialog(cslewedit).dtype = DLG_NOP
	dialog(ctiltedit).dtype = DLG_NOP
	dialog(tempedit).dtype = DLG_NOP
	dialog(xshiftedit).dtype = DLG_NOP
	dialog(mpfitmenu).dtype = DLG_NOP
	dialog(MPF_1).dtype = DLG_NOP
	dialog(MPF_2).dtype = DLG_NOP
	dialog(MPF_3).dtype = DLG_NOP
	dialog(MPF_4).dtype = DLG_NOP
	dialog(MPF_5).dtype = DLG_NOP
	dialog(MPF_6).dtype = DLG_NOP
	dialog(MPF_7).dtype = DLG_NOP
	dialog(MPF_8).dtype = DLG_NOP
	dialog(MPF_9).dtype = DLG_NOP
	dialog(MPF_10).dtype = DLG_NOP
	Do i=0,3
		dialog(pointmenus+i).dtype = DLG_NOP
		dialog(pointtags+i).dtype = DLG_NOP
	Enddo
	dialog(samplebutton).dtype = DLG_TEXT
	Do i=0,3
		dialog(cutbars+i).dtype = DLG_BOX
		dialog(cutbars+i).aux = 0
	Enddo
	dialog(15).dtype = DLG_SBAR
	dialog(15).aux = wind2d(curwin).inum2d
	dialog(CMD_PLAY).dtype = DLG_BUTTON
	dialog(CMD_LBL).dtype = DLG_TEXT
	dialog(CMD_EDIT).dtype = DLG_EDIT
	If (wind2d(curwin).mode .eq. 1) then
C
C	enable the stuff
C
		dialog(samplebutton).dtype = DLG_BUTTON
		Do i=0,3
			dialog(cutbars+i).dtype = DLG_SBAR
			dialog(cutbars+i).aux=wind2d(curwin).rots(i+1)
		Enddo
		dialog(15).dtype = DLG_BOX
		dialog(15).aux = 0
C
C	convert rots array to floats
C
		Do i = 1,3
			frots(i) = wind2d(curwin).rots(i)
		Enddo
		frots(4) = wind2d(curwin).rots(4)-256
		Call vl_sample_vectors(%val(imagesize),%val(imagesize),
     +			frots(1),frots(4),myset)
C
	Else if (wind2d(curwin).mode .eq. 2) then
C
C	Enable MP_FIT stuff
C
		dialog(thickedit).dtype = DLG_EDIT
		dialog(ipixeledit).dtype = DLG_EDIT
		dialog(countedit).dtype = DLG_EDIT
		dialog(offedit).dtype = DLG_EDIT
		dialog(cslewedit).dtype = DLG_EDIT
		dialog(ctiltedit).dtype = DLG_EDIT
		dialog(tempedit).dtype = DLG_EDIT
		dialog(xshiftedit).dtype = DLG_EDIT
		dialog(mpfitmenu).dtype = DLG_MENU
		dialog(samplebutton).dtype = DLG_BUTTON
		Do i=0,3
			dialog(pointmenus+i).dtype = DLG_MENU
			dialog(pointtags+i).dtype = DLG_TEXT
		Enddo
		dialog(MPF_1).dtype = DLG_TEXT
		dialog(MPF_2).dtype = DLG_TEXT
		dialog(MPF_3).dtype = DLG_TEXT
		dialog(MPF_4).dtype = DLG_TEXT
		dialog(MPF_5).dtype = DLG_TEXT
		dialog(MPF_6).dtype = DLG_TEXT
		dialog(MPF_7).dtype = DLG_TEXT
		dialog(MPF_8).dtype = DLG_TEXT
		dialog(MPF_9).dtype = DLG_TEXT
		dialog(MPF_10).dtype = DLG_TEXT
		dialog(CMD_PLAY).dtype = DLG_NOP
		dialog(CMD_LBL).dtype = DLG_NOP
		dialog(CMD_EDIT).dtype = DLG_NOP
		dialog(15).dtype = DLG_BOX
		dialog(15).aux = 0
C
C	Scroll bars
C
		dialog(cutbars+1).dtype = DLG_SBAR
		dialog(cutbars+1).aux = (rots.slew(2)+90.0)*2.0
		dialog(cuttext+1).text = "T slew"
		dialog(cuttext+1).tlength = 6
C
		dialog(cutbars+0).dtype = DLG_SBAR
		dialog(cutbars+0).aux = (rots.tilt(2)+90.0)*2.0
		dialog(cuttext+0).text = "T tilt"
		dialog(cuttext+0).tlength = 6
C
		dialog(cutbars+3).dtype = DLG_SBAR
		dialog(cutbars+3).aux = rots.uptrans(2) + 256.0
		dialog(cuttext+3).text = "Tab Y "
		dialog(cuttext+3).tlength = 6
C
		dialog(cutbars+2).dtype = DLG_SBAR
		dialog(cutbars+2).aux = rots.uptrans(3) + 180.0
		dialog(cuttext+2).text = "Tab Z "
		dialog(cuttext+2).tlength = 6
C
C	PET data
C
		write(dialog(pointtags+1).text,121)"Tot Slew:",
     +			rots.slew(2)+rots.slew(1)
		write(dialog(pointtags+0).text,121)"Tot Tilt:",
     +			rots.tilt(2)+rots.tilt(1)
		write(dialog(pointtags+3).text,120)"Tab Y:",
     +			rots.uptrans(2)*interpixel," mm"
		write(dialog(pointtags+2).text,120)"Tab Z:",
     +			rots.uptrans(3)*interpixel," mm"

120	Format(A6,F6.1,A3)
121	Format(A9,F6.1)
C
C	Setup arbitrary cutting plane stuff for tracing
C
		frots(4) = rots.sthickness/interpixel
		Do i=1,3
			frots(i) = rots.upcenter(i) +
     +                             float(-wind2d(curwin).n_slice)*
     +                             frots(4)*rots.upnorm(i)
		Enddo
		Call vl_mpsample_vectors(%val(imagesize),%val(imagesize),
     +			rots.upxaxis(1),rots.upyaxis(1),rots.upnorm(1),
     +			frots(1),myset)
	Endif
C
C	redraw
C
	If (who .eq. 0) Then
		Call qenter(REDRAW,0)
	Else if (who .eq. -1) Then
		Call qenter(REDRAW,wid)
	Else
		Call qenter(REDRAW,wid)
		Call qenter(REDRAW,who)
	Endif
C
	return
	End
C
C	routine to setup the lookup tables
C
	Subroutine setpal(palette,opacity,texture_c,texture_o,bcolor,myset)

	Implicit None

	Integer*4	palette(768),bcolor
	Integer*4	opacity(768)
	Integer*4	texture_c(768)
	Integer*4	texture_o(768)
	Integer*4	myset(*)

	Integer*4	llut(256),rlut(256)
	Integer*4	I,j,k
C
	Do I=1,256
		j = palette(i) + palette(i+256)*256 + 
     +			palette(i+512)*65536
		j = jior(j,jishft(opacity(i),24))
		rlut(i) = j
		j = texture_c(i) + texture_c(i+256)*256 + 
     +			texture_c(i+512)*65536
		j = jior(j,jishft(texture_o(i),24))
		llut(i) = j
	Enddo
C
	Call vl_lookups(rlut,llut,%val(bcolor),myset)
C
	return
	end
C
C	Routine to update the pointset menu
C
	Subroutine update_pset_menu(menu,start,prefix,lockout)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'

	Integer*4	menu,start,lockout
	Character*(*)	prefix
	Character*255	tmp,tempstr
	Integer*4	i,j,st

	Do i=0,19
		Call get_point_name(i,tmp,prefix)
		j = 200
		Do while (tmp(j:j) .eq. ' ')
			j = j - 1
		Enddo
		st = 1
		If (tmp(1:1) .eq. '#') st = 2
		If (start .eq. 3000) Then
			write(tempstr,18) tmp(st:j),I+start
18      Format("Load:",A," %x",I4.4,"}")
		Else if (start .eq. 4000) Then
			write(tempstr,17) tmp(st:j),I+start
17      Format("Save:",A," %x",I4.4,"}")
		Else
			write(tempstr,16) tmp(st:j),I+start
16      Format(A," %x",I4.4,"}")
		Endif
		Call change(menu,i+1,tempstr,index(tempstr,"}")-1,0)
		If ((lockout .ne. 0) .and. (st .eq. 2)) Then
			Call setpup(menu,i+1,PUPGRE)
		Else
			Call setpup(menu,i+1,PUPNON)
		Endif
	Enddo

	Return
	End
