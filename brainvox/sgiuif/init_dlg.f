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
C    MODULE NAME    : init_dlg and font_dlg
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
C    DESCRIPTION    :  	This routine sets up all the parameters for the
C			SGIUIF package... (init_dlg)
C			font_dlg - allows the user to change the 
C			bound fonts within SGIUIF...
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
	Subroutine init_dlg(status)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	status,state,j,k

	status=0
C
C	queue all devices we need
C
	Call Qdevic(MOUSE1)
	Call Qdevic(MOUSE3)
	Call Qdevic(KEYBD)
	Call qdevic(RIGHTA)
	Call qdevic(LEFTAR)
	Call qdevic(DELKEY)
	Call qdevic(RETKEY)
	Call qdevic(TABKEY)
C
C 	set up the fonts
C
	Call fminit
	Do J=1,10
		fonts(j) = 0
	Enddo
	cur_font = 1
	call f_fmsetfont(cur_font)
	status=0
C
C       find graphics state
C
        zbuffer=getzbu()
	state=getdis()
	if (state .eq. DMSING) Then
                RGBmode=.false.
                dbuffer=.false.
	Else if (state .eq. DMDOUB) Then
                RGBmode=.false.
                dbuffer=.true.
	Else if (state .eq. DMRGB) Then
                RGBmode=.true.
                dbuffer=.false.
	Else if (state .eq. DMRGBD) Then
                RGBmode=.true.
                dbuffer=.true.
	Endif
C
C	there is no current resource file...
C
	dlg_resunit=0
C
C	set default colors... (1,x)=red,(2,x)=blue,(3,x)=green 0...255
C
C	line color (white)
C
	dlg_colors(1,1)=255
	dlg_colors(2,1)=255
	dlg_colors(3,1)=255
C
C	fill color (gray)
C	
	dlg_colors(1,2)=128
	dlg_colors(2,2)=128
	dlg_colors(3,2)=128
C
C	invert color (green)
C
	dlg_colors(1,3)=0
	dlg_colors(2,3)=100
	dlg_colors(3,3)=0
C
C	button color (blue)
C	
	dlg_colors(1,4)=0
	dlg_colors(2,4)=0
	dlg_colors(3,4)=100
C
C	button feedback color (red)
C
	dlg_colors(1,5)=100
	dlg_colors(2,5)=0
	dlg_colors(3,5)=0
C
C	scroll bar fill color (off blue)
C	
	dlg_colors(1,6)=0
	dlg_colors(2,6)=80
	dlg_colors(3,6)=100
C
C	text color  (white)
C
	dlg_colors(1,7)=255
	dlg_colors(2,7)=255
	dlg_colors(3,7)=255
C
C	background color (black)
C	
	dlg_colors(1,8)=0
	dlg_colors(2,8)=0
	dlg_colors(3,8)=0
C
C	set the map if in color mapping mode...
C	use indexes (cbase - cbase+7)
C	cbase must protect  0-255 and 3840-4095 for non-GT systems
C	on GT systems this is not needed.
C
	If (.not. RGBmode) then
		Do state=1,8
			Call mapcol(state+cbase-1,dlg_colors(1,state),
     +  			dlg_colors(2,state),dlg_colors(3,state))
		Enddo
	Endif
C
	Return
	End
C
C---------------------------------------------------------------------------
C---------------------------------------------------------------------------
C
C	This is the font changing routine:
C	There are 10 fonts under SGIUIF numbered 1-10
C
C	font#  use
C	  1	Font to draw dialog item text (labels)
C	  2     Font to draw EDIT and TEXT item text
C	 3-10   Alternate fonts for individual EDIT and TEXT items
C
C	In DLG_EDIT and DLG_TEXT items the AUX field has been
C	futher enhanced to use the alternate fonts:
C	In a DLG_TEXT item:
C	AUX bits- 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
C		  |  |  |  |  reserved    |  |  |  |  |  |  |  |
C                 |  |  |  |              \--------------------/
C                 |  |  \--/            FONT number to use (if 0 then use 2)
C                 |  |   Justification (10-left  00-center 01-right)
C                 |  \- Framing bit (1-on)
C                 \- Background bit (1-invert backgound)
C
C	In a DLG_EDIT item the 0bit is the framing bit and 1bit is undef
C       DLG_EDIT only supports left justification
C
C	This routine loads the font given in fontname at points size
C	into font number index.  Status=0 if no error.
C
	Subroutine	font_dlg(fontname,points,idex,status)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	character*(*)	fontname
	character*200	name
	real*4		points
	integer*4	idex,tmpfont
	Integer*4	status,state,i(5),k
C
C	assume no error...
C
	status=0
C
	if (points .lt. 0.5) then  !no smaller than 1/2 point
		status=-1
		return
	Endif
C
	If ((idex .lt. 1).or.(idex .gt. 10)) then
		status=-1
		return
	Endif
C
C	find the end of the font name
C	
	k=index(fontname," ")
	if (k .gt. 199) then
		status=-1
		return
	Endif
C
C	free up the old font
C
	Call fmfreefont(fonts(idex))
C
C	load the new
C
	name=fontname(1:k-1)//char(0)
	Call fmfindfont(name,tmpfont)
C
C	set the points
C
	Call fmscalefont(tmpfont,points,fonts(idex))
C
C	free up the tmp font
C
	Call fmfreefont(tmpfont)        
C
C	retrieve font metrics info
C
	Call fmgetfontinfo(fonts(idex),i(1),i(2),i(3),i(4),i(5))
	textdecend(idex) = i(2)
	textheight(idex) = i(5)  ! actual height...(no blank space)
C	
	return
	End
C
C	A "smart" font setting routine
C
	Subroutine	f_fmsetfont(j)
	
	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  
	
	Integer*4	k,j,state,i(5)

	Character*20	fontlist(10)
	Real*4		pointlist(10)

	data fontlist/	"Screen-Bold ",
     +			"Screen-Bold ",
     +			"Symbol ",
     +			"Symbol ",
     +			"Helvetica ",
     +			"Helvetica ",
     +			"Times-Roman ",
     +			"Times-Roman ",
     +			"Times-Roman ",
     +			"Times-Roman "/

	data pointlist/	12.0,
     +			12.0,
     +			12.0,
     +			14.0,
     +			18.0,
     +			14.0,
     +			12.0,
     +			14.0,
     +			18.0,
     +			24.0/

	If (fonts(j) .eq. 0) Then
		k=index(fontlist(j)," ")
		Call fmfindfont(fontlist(j)(1:k-1)//char(0),state) !get handle
		If (state .eq. 0) then
			fonts(j) = fonts(1)
			write(*,*) "Could not find font:",fontlist(j)(1:k-1)
		Else
			Call fmscalefont(state,pointlist(j),fonts(j))  !set points
			Call fmfreefont(state)  ! free the temp font handle
		Endif
		Call fmgetfontinfo(fonts(j),i(1),i(2),i(3),i(4),i(5))
		textdecend(j) = i(2)
		textheight(j) = i(5)  ! actual height...
	Endif

	If (fonts(j) .ne. 0) Call fmsetfont(fonts(j))

	Return
	End
