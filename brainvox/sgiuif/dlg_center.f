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
C    MODULE NAME    : dlg_center
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
C    DESCRIPTION    :  	this routine will center some text in a bounding box
C			and draw it.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call  dlg_center(x1,y1,x2,y2,text,tlen)
C    INPUTS         : x1,x2,y1,y2 : rectangle
C		      text : the text to  draw
C		      tlen : the length of the text         
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  dlg_center(x1,y1,x2,y2,text,tlen)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	x1,x2,y1,y2,xr,yr,tlen
	Character*200	text
C
	Integer*4	len,dx,dy
C
	Integer*4	fmwid
	external	fmwid
C
C	find dx,dy and textlength
C
	dx=x2-x1
	dy=y1-y2
	len=fmwid(text,tlen)
C
	xr=x1+(dx-len)/2
	yr=y2+(dy-textheight(cur_font))/2+textdecend(cur_font)
C
	Call cmov2i(xr,yr)
	Call f_fmsetfont(cur_font)
	Call fmprstr(text(1:tlen)//char(0))
	
	Return
	End
C
C	new string width function using FM calls
C
	Integer*4	Function 	fmwid(text,len)

	Implicit None

	Include 'sgiuif.f'

	Character*(*)	text
	Character*200 	text2
	Integer*4	len,width

	text2 = text(1:len)//char(0)

	call fmgetstrwidth(fonts(cur_font),text2,width)

	fmwid = width

	Return
	End
