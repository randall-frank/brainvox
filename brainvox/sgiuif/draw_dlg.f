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
C    MODULE NAME    : draw_dlg
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
C    DESCRIPTION    :  	This routine draws the passed dialog 
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
	Subroutine draw_dlg(dialog,status)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	record	/ditem/	dialog(*)
	Integer*4	status
	
	Integer*4	I  !index into array...
	Integer*4	r(4)
	Logical		backg  !is r() to be painted index8?
	Logical		fedit  ! do I need to set the current edit field
	integer*2	l,t,b,ri
C
C	assume the best
C
	status=0
C
C	get the conversion from coords to pixels
C
	Call getvie(l,ri,b,t)
C
        Do I=1,4
                dlgrect(I)=dialog(1).rect(I)
        Enddo
C
	cortopix=float(dlgrect(3)-dlgrect(1))/float(ri-l)
C
C	start at the top...
C
	I=1
C
C	is it a header???
C
	If (dialog(1).dtype .ne. DLG_HEADER) then
		Status=-1
		Return
	Endif
C
C	is the background to be painted???
C
	if (dialog(1).aux .eq. 0) then
		backg=.true.
		r(1)=dialog(1).rect(1)
		r(2)=dialog(1).rect(2)
		r(3)=dialog(1).rect(3)
		r(4)=dialog(1).rect(4)
	Else
		backg=.false.
	Endif
C
C	fedit is true if the package is to find&set the current edit field
C
	If (dialog(1).tpos .le. 0) then
		fedit=.true.
	Else
		fedit=.false.
	Endif
C
C	init the colors to the base ones. (dialog(1) is a dummy)
C
	Call dlg_d_color(dialog(1),1)
C
C	do all the items in the dialog...
C
	Do while (dialog(I).dtype .ne. DLG_END)
		I=I+1
C
C	if the background is to be painted and the current item
C	is not changing the color, Paint it and remove flag.
C
		If ((backg) .and. (dialog(I).dtype .ne. DLG_COLOR)) Then
			If (RGBmode) then
				Call c3i(dlg_colors(1,8))
			Else
				Call color(cbase+8)
			Endif
			Call rectfi(r(1),r(2),r(3),r(4))
			backg=.false.
		Endif
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
		Call dlg_d_color(dialog(I),0)
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
		Call dlg_d_line(dialog(I))
		Goto 1300
C
C	DLG_BOX
C
500		Continue
		Call dlg_d_box(dialog(I))
		Goto 1300
C
C	DLG_TEXT 
C
600		Continue
		Call dlg_d_text(dialog(I))
		Goto 1300
C
C	DLG_EDIT 
C
700		Continue
C
C	if we are looking for an active one, here it is
C
		If (fedit) then
			fedit=.false.
			dialog(1).tpos=I
		Endif
C
C	select if it is active or not
C
		If (dialog(1).tpos .eq. I) then
			Call dlg_d_edit(dialog(I),1)
		Else
			Call dlg_d_edit(dialog(I),0)
		Endif
		Goto 1300
C
C	DLG_SBAR 
C
800		Continue
		Call dlg_d_sbar(dialog(I),0)
		Goto 1300
C
C	DLG_MENU 
C
900		Continue
		Call dlg_d_menu(dialog(I),0)
		Goto 1300
C
C	DLG_CHECK 
C
1000		Continue
		Call dlg_d_checkbox(dialog(I),0)
		Goto 1300
C
C	DLG_RADIO 
C
1213		Continue
		Call dlg_d_radio(dialog(I),0)
		Goto 1300
C
C	DLG_BUTTON
C
1100		Continue
		Call dlg_d_button(dialog(I),0)
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
	Call gflush
C
	Status=0
C	
	Return
	End
