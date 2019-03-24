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
C    MODULE NAME    : filenames
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 10 Mar 01       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	 routines to handle keyboard modifier menus.
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
C
	Subroutine keyboard_do_menu(value)

	Implicit None

	Include	'keyboard_inc.f'

	Integer*4	value

	Integer*4	keymenu,key_shift,key_alt,key_ctrl
	Common		/keyboard/keymenu,key_shift,key_alt,key_ctrl

	If (value .eq. KEYBOARD_SHIFT) Then 
		key_shift = 1 - key_shift
	Else if (value .eq. KEYBOARD_CTRL) Then
		key_ctrl = 1 - key_ctrl
	Else if (value .eq. KEYBOARD_ALT) Then
		key_alt = 1 - key_alt
	Endif

	Return
	End

	Integer*4 Function keyboard_query(which)

	Implicit None

	Include	'keyboard_int.f'

	Integer*4	which

	Integer*4	keymenu,key_shift,key_alt,key_ctrl
	Common		/keyboard/keymenu,key_shift,key_alt,key_ctrl

	If (which .eq. KEYBOARD_SHIFT) keyboard_query = key_shift
	If (which .eq. KEYBOARD_CTRL) Then 
		keyboard_query = key_ctrl
	Endif
	If (which .eq. KEYBOARD_ALT) Then 
		keyboard_query = key_alt
	Endif

	Return
	End

	Subroutine keyboard_create_menu(menu)

	Implicit None

	Include	'keyboard_inc.f'

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
C
	Integer*4	menu
	Character*256	tempstr
C
	Integer*4	keymenu,key_shift,key_alt,key_ctrl
	Common		/keyboard/keymenu,key_shift,key_alt,key_ctrl
	Data		keymenu/-1/
	Data		key_shift/0/
	Data		key_alt/0/
	Data		key_ctrl/0/
C
	If (keymenu .eq. -1) Then 
		keymenu = newpup()
        	tempstr="Shift %s%x29870%i|Ctrl %x29871%i|Alt %x29872%i}"
        	Call addtop(keymenu,tempstr,index(tempstr,"}")-1,0)
	Endif

        tempstr="Keyboard %s%m}"
        Call addtop(menu,tempstr,index(tempstr,"}")-1,keymenu)

	Return
	End
