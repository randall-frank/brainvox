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
C    MODULE NAME    : setbysliders
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_trace
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine sets the image translation by the
C			positions of the sliders...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call setbysliders(xo,yo,state,trace)
C    INPUTS         :          state : program state
C				trace : current trace for update..
C    OUTPUTS        :  xo,yo : x,y of the image pixel to appear in the
C				lower left corner of the display
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine setbysliders(xo,yo,state,trace)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	xo,yo,state,trace(4096,2)
	Integer*4	I,J
	Logical		slide
C
C	validate zoom
C
	Do while (zoom*512 .lt. wx) 
		zoom = zoom + 1
	Enddo
	Do while (zoom*512 .lt. wy) 
		zoom = zoom + 1
	Enddo
C
C	get slider values...
C
	If (state .eq. 1) then
		I=state1(3).aux+1     !(1-512)
		J=state1(4).aux+1
	Else
		I=state2(3).aux+1     !(1-512)
		J=state2(4).aux+1
	Endif
C
C	invert the vertical slider...
C
	J=513-j
C
C	convert from scroll bar values into xo,yo values
C
	I=I-(wx/2)/zoom
	J=J-(wy/2)/zoom
C
C	bracket to stops if needed...
C
	slide=.false.
	If (I .lt. 1) then 
		slide=.true.
		I=1
	Endif
	If (I .gt. (512*zoom-wx)/zoom) then
		slide=.true.
		 I=(512*zoom-wx)/zoom
	Endif
	If (j .lt. 1) then
		slide=.true.
		j=1
	Endif
	If (j .gt. (512*zoom-wy)/zoom)  then
		slide=.true.
		j=(512*zoom-wy)/zoom
	Endif
C
C	the values are now OK for xo,yo (1-512)
C
	xo=I
	yo=J
C
C	set the scroll bar values... (remember to invert the Y)
C
	If (state .eq. 1) then
		state1(3).aux=(I+(wx/2)/zoom)-1
		state1(4).aux=511-((j+(wy/2)/zoom)-1)
	Else
		state2(3).aux=(I+(wx/2)/zoom)-1
		state2(4).aux=511-((j+(wy/2)/zoom)-1)
	Endif
C
C	redraw the screen
C
	Call doimage(xo,yo,state,trace)
C
C	If the sliders changed; redraw them...
C
C	If (slide) Call dodlg(state)
C	
	Return
	End
