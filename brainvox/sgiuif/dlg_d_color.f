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
C    MODULE NAME    : dlg_d_color
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
C    DESCRIPTION    :  	Routine to set the color map
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call dlg_d_color(item,state)
C    INPUTS         :          Item: DLG_COLOR item
C			       state : use the ITEM or not...
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine dlg_d_color(item,state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	I,state,J    
	Record /ditem/	item
C
C	parse the colors from the item...
C
C	if state =0 then get the colors and send them
C	If state =1 then send the current colors only...
C
	If (state .eq. 0) then
	    Read(Unit=item.text,fmt=10)((dlg_colors(J,I),J=1,3),I=1,8)
10	Format(24I3)  !8*3*RRRGGGBBB
	Endif
C
C	set the map if in color mapping mode...
C	use indexes (cbase+1 - cbase+8)
C	cbase must protect  0-7 and 3840-4095 for non-GT systems
C	on GT systems this is not needed.
C
	If (.not. RGBmode) then
		Do I=1,8
			Call mapcol(I+cbase,dlg_colors(1,I),
     +  			dlg_colors(2,I),dlg_colors(3,i))
		Enddo
	Endif
C
	Return
	End
