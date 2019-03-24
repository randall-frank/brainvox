*
************************************************
* Local include file for the SGIUIF package    *
* Copyright (C) 1989 Image Analysis Facility   *
*               71 HBRF University of Iowa     *
************************************************
*
*	This is the local include file to the sgiuif package
*	It is to be included only in the package itself, not
*	the user programs...
*
	Integer*4	max_fonts
	parameter	(max_fonts = 10)
*
	Integer*4	dlg_colors(3,8)
	Character*200	dlg_resfile
	Integer*4	dlg_resunit
	Logical		zbuffer
	Logical		dbuffer
	Logical		RGBmode
	Integer*4	textheight(max_fonts)
	Integer*4	textdecend(max_fonts)
	Integer*4	fonts(max_fonts),cur_font
	Integer*4 	dlgrect(4)  !left,top,right,bottom
	real*4		cortopix
*
	Common	/DLG_COMMON_BLOCK/dlg_colors,dlg_resfile,dlg_resunit,
     +		zbuffer,dbuffer,RGBmode,textheight,dlgrect,cortopix,
     +		textdecend,fonts,cur_font
*
*	define the base index for color map mode 
*	sgiuif uses color indexes (cbase+1) to (cbase+7)
*
	Integer*4	cbase
	Parameter	(cbase = 23)
*
*	modification for IRIX 3.2 colormap...
*	The cbase colors overwrote the pastels used by 3.2
*	(indexes 8-16)  We will now use the colors reserved for X
*	(upper 8 colors indexes 24-32)
*
*
******* End of sgiuif local include file *******
*
