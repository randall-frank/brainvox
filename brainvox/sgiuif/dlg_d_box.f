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
C    MODULE NAME    : dlg_d_box
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
C    DESCRIPTION    :  	This routine will draw a box filled &/^ outlined
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_box(item)
C    INPUTS         :          item.rect, item.aux
C    OUTPUTS        : none
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_d_box(item)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Record	/ditem/	item
	Integer*4	I
C
C	filled
C
	If (item.aux .gt. 0) then
		I=2
		If (item.tpos .ne. 0) I=item.tpos
		If (RGBmode) then
			Call c3i(dlg_colors(1,I))
		Else
			Call color(I+cbase)
		Endif
		Call rectfi(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
	Endif
C
C	outline
C
	If (item.aux .ne. 1) then
		If (RGBmode) then
			Call c3i(dlg_colors(1,1))
		Else
			Call color(1+cbase)
		Endif
		Call recti(item.rect(1),item.rect(2),item.rect(3),
     +			item.rect(4))
	Endif

	Return
	End
