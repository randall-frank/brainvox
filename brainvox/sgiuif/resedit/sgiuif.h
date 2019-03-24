************************************************
* Include file for the SGIUIF package          *
* Copyright (C) 1989 Image Analysis Facility   *
*               71 HBRF University of Iowa     *
************************************************
*
*	version number
*
	Real*4   	DLG_VERSION
	Parameter	(DLG_VERSION = 1.10)
*
*	The dialog item structure...
*
	Structure	/ditem/
		Integer*4	dtype
		Integer*4	tlength
		Integer*4	tpos
		Character*80	text
		Integer*4	rect(4)
		Integer*4	aux
	End Structure
*
*	Define the item types...
*
	Integer*4	DLG_HEADER
	Parameter	(DLG_HEADER = 1)
	Integer*4	DLG_COLOR 
	Parameter	(DLG_COLOR  = 2)
	Integer*4	DLG_END   
	Parameter	(DLG_END    = 3)
	Integer*4	DLG_LINE  
	Parameter	(DLG_LINE   = 4)
	Integer*4	DLG_BOX   
	Parameter	(DLG_BOX    = 5)
	Integer*4	DLG_TEXT  
	Parameter	(DLG_TEXT   = 6)
	Integer*4	DLG_EDIT  
	Parameter	(DLG_EDIT   = 7)
	Integer*4	DLG_SBAR  
	Parameter	(DLG_SBAR   = 8)
	Integer*4	DLG_MENU  
	Parameter	(DLG_MENU   = 9)
	Integer*4	DLG_CHECK 
	Parameter	(DLG_CHECK  = 10)
	Integer*4	DLG_BUTTON 
	Parameter	(DLG_BUTTON = 11)
	Integer*4	DLG_NOP    
	Parameter	(DLG_NOP    = 12)
	Integer*4	DLG_RADIO  
	Parameter	(DLG_RADIO  = 13)
*
*	define the event types...
*
	Integer*4	DLG_CHKEVNT
	Parameter	(DLG_CHKEVNT = 3000)
	Integer*4	DLG_BUTEVNT
	Parameter	(DLG_BUTEVNT = 3001)
	Integer*4	DLG_BAREVNT
	Parameter	(DLG_BAREVNT = 3002)
	Integer*4	DLG_EDTEVNT
	Parameter	(DLG_EDTEVNT = 3003)
	Integer*4	DLG_POPEVNT
	Parameter	(DLG_POPEVNT = 3004)
	Integer*4	DLG_TXTEVNT
	Parameter	(DLG_TXTEVNT = 3005)
	Integer*4	DLG_NOPEVNT
	Parameter	(DLG_NOPEVNT = 3006)
	Integer*4	DLG_RADEVNT
	Parameter	(DLG_RADEVNT = 3007)
*
******* end of SGIUIF includes **************
