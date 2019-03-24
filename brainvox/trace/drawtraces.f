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
C    MODULE NAME    : drawtraces
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
C    DESCRIPTION    :  	This routine draws the traces into their respective
C			planes...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call drawtraces(sel,trace,aux,xo,yo)
C    INPUTS         :          sel : what traces to use (back or forg)
C			trace : The alternative (forg) roi
C			aux : what to do.  (erase or draw)
C			xo,yo : current scroll of the image...
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 

	Subroutine drawtraces(sel,trace,aux,xo,yo,fbuffer)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	Include 'tr_windows.inc'

	Integer*4	aux,xo,yo
	Integer*4	trace(4096,2),I,J,stop
	Integer*4	t_trace(4096,2),usergb,wtmp,h
	logical		sel,fbuffer
C
	real*4		p1(2),p2(2),part(2)

        Integer*4       grid_color
        Real*4          grid_cm
        common          /grid_stuff/grid_color,grid_cm
C
	wtmp = winget()
C
	call winset(wid2)
	Call reshap
C
	If (fbuffer) Then
		call frontb(.true.)
	Endif
C
C       Get visual mode
C
        If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then
                usergb = 1
        Else
                usergb = 0
        Endif
C
C	if sel=.true. then use TRACE and current bit
C	if sel=.false. then use /global/ and background bit
C
C	if aux=-1 then erase the plane
C	if aux= 0 then erase that trace
C	if aux= 1 then draw it
C
C	set line width
C
	if (exact_tracing .eq. 0) Call linewi(zoom)
C
C	set masking planes...
C
	If (sel) then
C	current  (bit 11)        
		If (usergb .eq. 1) Then
			Call cpack(cpalette(258))
		Else
			h = '00000400'X
			Call writem(h)
			h = '00000500'X
			Call color(h)
		Endif
	Else
C	background   (bit 10)
		If (usergb .eq. 1) Then
			Call cpack(cpalette(257))
		Else
			h = '00000200'X
			Call writem(h)
			h = '00000300'X
			Call color(h)
		Endif
	Endif
C
C	set the stopping point for color drawing
C
	If (aux .lt. -1) then
		stop = -(aux+1)
	Else
		stop = 99999
	Endif
C
C	if aux=-1 then nuke the plane...
C
	If (aux .eq. -1) then
		If (usergb .eq. 1) Then
C			Call qenter(REDRAW,wid2)
		Else
			Call color(0)
			Call rectfi(0,0,wx,wy)
		Endif
	Else
C
C	if (erase) then set NULL color 
C
		If (aux .eq. 0) Then
			If (usergb .eq. 1) Then
				Call cpack(0)
			Else
				Call color(0)
			Endif
		Endif
C
C	color is set... let's draw...
C
		part(1)=0.49*float(zoom)
		part(2)=0.49*float(zoom)
		If (sel) then
C
C	draw edit x's if they are selected...
C
		   if ((editstate .gt. 1).and.(editstate .ne. 5)) then
			Do I=1,editstate-1
C
C	draw an X...
C
				p1(1)=float((edit(I,1)+2-xo)*zoom)+part(1)
				p1(2)=float((edit(I,2)+2-yo)*zoom)+part(2)
				p2(1)=float((edit(I,1)-2-xo)*zoom)+part(1)
				p2(2)=float((edit(I,2)-2-yo)*zoom)+part(2)
				Call bgnlin
				Call v2f(p1)
				Call v2f(p2)
				Call endlin
C
				p1(1)=float((edit(I,1)-2-xo)*zoom)+part(1)
				p1(2)=float((edit(I,2)+2-yo)*zoom)+part(2)
				p2(1)=float((edit(I,1)+2-xo)*zoom)+part(1)
				p2(2)=float((edit(I,2)-2-yo)*zoom)+part(2)
				Call bgnlin
				Call v2f(p1)
				Call v2f(p2)
				Call endlin
			Enddo
		   Endif
	If (exact_tracing .eq. 0) Then
C
		   if (trace(1,1) .gt. 1) then
C
C	draw the line...
C
			p2(1)=float((trace(2,1)-xo)*zoom)+part(1)
			p2(2)=float((trace(2,2)-yo)*zoom)+part(2)
			Do I=3,trace(1,1)+1
				stop = stop - 1
				If (stop .le. 0) Then
					If (usergb .eq. 1) Then
						Call cpack(0)
					Else
						Call color(0)
					Endif
				Endif
				p1(1)=p2(1)
				p1(2)=p2(2)
				p2(1)=float((trace(I,1)-xo)*zoom)+part(1)
				p2(2)=float((trace(I,2)-yo)*zoom)+part(2)
				Call bgnlin
			 	Call v2f(p2)
			 	Call v2f(p1)
				Call endlin
			Enddo
C
		   Elseif (trace(1,1) .eq. 1) then
C
C	a single point...
C
			p2(1)=float((trace(2,1)-xo)*zoom)+part(1)
			p2(2)=float((trace(2,2)-yo)*zoom)+part(2)
			Call bgnpoi
			Call v2f(p2)
			Call endpoi
		   Endif
	Else
			Call draw_roi(trace,zoom,stop,0,xo,yo,usergb)
	Endif
C
		Else
C
C	handle the background rois case...
C
		    Do J=1,numrgns 
		      if (background(J).and.(rois(J,1,1) .gt. 1)) then
	If (exact_tracing .eq. 0) Then
C
C	insure closure...
C
			I=rois(J,1,1)+1
			p1(1)=float((rois(j,I,1)-xo)*zoom)+part(1)
			p1(2)=float((rois(j,I,2)-yo)*zoom)+part(2)
			p2(1)=float((rois(j,2,1)-xo)*zoom)+part(1)
			p2(2)=float((rois(j,2,2)-yo)*zoom)+part(2)
			Call bgnlin
			call v2f(p1)
			call v2f(p2)
			Call endlin
C
C	loop around...
C
			Do I=3,rois(j,1,1)+1
			   p1(1)=p2(1)
			   p1(2)=p2(2)
			   p2(1)=float((rois(j,I,1)-xo)*zoom)+part(1)
			   p2(2)=float((rois(j,I,2)-yo)*zoom)+part(2)
			   Call bgnlin
			   Call v2f(p1)
		 	   Call v2f(p2)
			   Call endlin
			Enddo
	Else
			t_trace(1,1) = rois(j,1,1)
			Do I=2,rois(j,1,1)+1
				t_trace(i,1) = rois(j,i,1)
				t_trace(i,2) = rois(j,i,2)
			Enddo
			stop = 99999
			Call draw_roi(t_trace,zoom,stop,1,xo,yo,usergb)
	Endif
 		      Endif
C
C	next background ROI
C
		    Enddo
C
C	draw RIGHT/LEFT text
C
		    Call cmov2i(10,10)
		    Call charst("RIGHT",5)
		    Call cmov2i(wx-50,10)
		    Call charst("LEFT",4)
C
C	Draw a grid?
C
		    If (grid_cm .gt. 0) Then
                        h = 'ffffffff'X
			If (usergb .eq. 0) Call writem(h)
			Call linewi(1)
			Call draw_grid(zoom*2,grid_color,grid_cm)
		    Endif
C
		Endif
	Endif
C
C	fix masking planes... (lower 12 bits)
C	and linewidth and recall the saved viewport (read screenmask)
C
	If (usergb .eq. 0) Then
                h = 'ffffffff'X
		Call writem(h)
		Call color(8)
	Endif
	if (exact_tracing .eq. 0) Call linewi(1)
C
	call gfflush
	If (fbuffer) Then
		call frontb(.false.)
	Endif
C
	Call winset(wtmp)
C
	Return
	End
C
	Subroutine draw_roi(trace,zoom,stop,closed,xo,yo,usergb)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'

	Integer*4	trace(4096,2),I,stop,closed
	Integer*4	zoom,p(4),xo,yo,usergb

	Do I=2,trace(1,1)+1
                If (stop .le. 0) Then
			If (usergb .eq. 1) Then
				Call cpack(0)
			Else
				Call color(0)
			Endif
		Endif
		stop = stop - 1
		p(1) = trace(I,1)/2
		p(2) = trace(I,2)/2
		p(1) = ((p(1)*2-xo+1)*zoom)
		p(2) = ((p(2)*2-yo+1)*zoom)
		p(3) = p(1) + (zoom*2)-1
		p(4) = p(2) + (zoom*2)-1
		Call sboxfi(p(1),p(2),p(3),p(4))
	Enddo

	Return
	End
