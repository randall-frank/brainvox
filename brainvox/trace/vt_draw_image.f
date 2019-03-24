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
C    MODULE NAME    : tr_draw_image
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
C    DESCRIPTION    :  	This routine renders the screen image..
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call tr_draw_image(x,y)
C    INPUTS         :          x,y : is the pixel to be in the LL corner
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_draw_image(x,y)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	Include 'vt_arb_inc.f'
C
	Integer*4	x,y
C
	Integer*4	rl,ys,j,i,k,rh,h
	Integer*2	holder(512*512)
	Integer*4	holder4(512*512)
	Real*4		izoom,temp
	Integer*4       slicetoZ,error,out(4)
C
        Integer*4       num2dpts,w2dpts(4096,4),p(4)
C
	External        Slicetoz
C
        common          /fvox/num2dpts,w2dpts
	common		/tr_buffers/holder,holder4

        Integer*4       col(14),usergb

        data    col/'00000000'X,
     +              '000000ff'X,
     +              '0000ff00'X,
     +              '0000ffff'X,
     +              '00ff0000'X,
     +              '00ff00ff'X,
     +              '00ffff00'X,
     +              '00ffffff'X,
     +              '000080ff'X,
     +              '0000ff80'X,
     +              '0080ff00'X,
     +              '00ff8000'X,
     +              '00ff0080'X,
     +              '008000ff'X/
C
C       Get visual mode
C
        If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then
                usergb = 1
        Else
                usergb = 0
        Endif
C
C	Calc The width (and the height) of the rectangle to bitblast
C
	rl=(wx/zoom)
	rh=(wy/zoom)
C
C	fill in the holder array with the rectangle of the image to be
C	blasted...  This is faster than sending the image a line at a time
C	as was done before... (CPU is cheap... very cheap)
C
	If (usergb .eq. 1) Then
		K=1
		If (PD_l) then
			Do J=y,y+rh-1
				Do I=x,x+rl-1
			holder4(k)=cpalette(PD(j*512+I-512)-255)
					k=k+1
				Enddo
			Enddo
		Else
			Do J=y,y+rh-1
				Do I=x,x+rl-1
			holder4(k)=cpalette(T2(j*512+I-512)-255)
					k=k+1
				Enddo
			Enddo
		Endif
C
C	set the zoom factor and  send the pixels..
C
		Call rectzo(float(zoom),float(zoom))
		Call lrectw(0,0,rl-1,rh-1,holder4(1))
C
	Else
		K=1
		If (PD_l) then
			Do J=y,y+rh-1
				Do I=x,x+rl-1
					holder(k)=PD(j*512+I-512)
					k=k+1
				Enddo
			Enddo
		Else
			Do J=y,y+rh-1
				Do I=x,x+rl-1
					holder(k)=T2(j*512+I-512)
					k=k+1
				Enddo
			Enddo
		Endif
C
C	Set  the write mask to the image bitplanes 
C	set the zoom factor and  send the pixels..
C
		h = '000001ff'X
		Call writem(h)
		Call rectzo(float(zoom),float(zoom))
		Call rectwr(0,0,rl-1,rh-1,holder(1))
C
	Endif
C
C	fix the zoom...
C
	Call rectzo(1.0,1.0)
C
C	Paint edges
C
	If (usergb .eq. 1) Then
		Call cpack(0)
	Else
		call color(BLACK)
	Endif
	Call sboxfi(rl*zoom,0,(rl+1)*zoom,1000)
	Call sboxfi(0,rh*zoom,1000,(rh+1)*zoom)
C
C	if points exist then draw them
C	
	izoom = (512/imagesize)
	temp = float(slicetoz(2) - slicetoz(1))/2.0
	if (temp .gt. 1.7) temp = 1.7
	Do i=1,num2dpts
C
	    If (arb_mode .eq. 0) Then
C
                if (abs(slicetoz(cur_slice)-w2dpts(I,3)) .le. temp) then
C
			If (usergb .eq. 0) Then
                        	Call set_color_index(col(w2dpts(I,4)+1))
			Else
				Call cpack(col(w2dpts(I,4)+1))
			Endif
                        p(1) = (((w2dpts(I,1)*izoom)-x+1)*zoom)
                        p(2) = (((w2dpts(I,2)*izoom)-y+1)*zoom) 
                        p(3) = p(1) + (zoom*2)-1
                        p(4) = p(2) + (zoom*2)-1
			if (p(1) .ge. wx) goto 10
			if (p(1) .lt. 0) goto 10
			if (p(2) .ge. wy) goto 10
			if (p(2) .lt. 0) goto 10
                        Call sboxfi(p(1),p(2),p(3),p(4))
10			Continue
C
		Endif
C
	    Else
C
		p(1) = w2dpts(I,1)
		p(2) = w2dpts(I,2)
		p(3) = w2dpts(I,3)
		Call vl_3dtocutplane(p(1),out(1),temp,error)
C#ifdef DEBUG
C        type *,"Paintpoints:in",p(1),p(2),p(3),"out:",out(1),out(2),out(3)
C#endif
                If (error .eq. 0) then
			If (usergb .eq. 0) Then
                        	Call set_color_index(col(w2dpts(I,4)+1))
			Else
				Call cpack(col(w2dpts(I,4)+1))
			Endif
                        p(1) = (((out(1)*izoom)-x+1)*zoom)
                        p(2) = (((out(2)*izoom)-y+1)*zoom) 
                        p(3) = p(1) + (zoom*2)-1
                        p(4) = p(2) + (zoom*2)-1
			if (p(1) .ge. wx) goto 20
			if (p(1) .lt. 0) goto 20
			if (p(2) .ge. wy) goto 20
			if (p(2) .lt. 0) goto 20
                        Call sboxfi(p(1),p(2),p(3),p(4))
20			Continue
			
		Endif

	    Endif
	Enddo
C
C	clean up the writemask and exit..
C
	h = 'ffffffff'X
	If (usergb .eq. 0) Call writem(h)
C
	Return
	End
