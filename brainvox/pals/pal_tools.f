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
C    MODULE NAME    : Pal_tools
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_pals
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 15 Mar 94       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to implement the palette editting functions
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
	Subroutine	lut2edit(luts,state,st,en)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'pals.inc'

	Integer*4	i,luts(3,256,4),state,st,en,p1,p2
	Real*4		r,g,b,a,h,s,v
C
C	convert into the current colormode and copy into the ed_pal
C
	If (en .lt. st) Then
		p1 = en
		p2 = st
	Else
		p1 = st
		p2 = en
	Endif
	Do i=p1,p2
		r = float(luts(state,i,1))/255.0
		g = float(luts(state,i,2))/255.0
		b = float(luts(state,i,3))/255.0
		a = float(luts(state,i,4))/255.0
		If (color_model .eq. 2) Then
			Call RGB_to_HSV(%val(r),%val(g),%val(b),h,s,v)
			ed_pal(i,1) = h
			ed_pal(i,2) = s
			ed_pal(i,3) = v
			ed_pal(i,4) = a
		Else
			ed_pal(i,1) = r
			ed_pal(i,2) = g
			ed_pal(i,3) = b
			ed_pal(i,4) = a
		Endif
	Enddo
C
	Return
	End
C
	Subroutine	bitslice_palette(luts,vol,bits,scaled,min,max)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'pals.inc'

	Integer*4       i,luts(3,256,4),vol,bits,scaled,min,max
	Integer*4	steps,j,z
	Real*4		f,tlut(256)
C
C	Type *,"b,s,v,max,min=",bits,scaled,vol,max,min
C
C	special cases
C
	If (bits .le. 0) Return
	If (bits .ge. 8) Return
	If (scaled .eq. 0) Then
		min = 0
		max = 255
	Endif
C
C	number of steps
C
	steps = 2**(8-bits)
	f = (max-min)/float(steps)

	Do z=1,4
		Do i=1,256
			tlut(i) = ed_pal(i,z)
		Enddo
		Do i=0,255
			j = iand(steps-1,i)
			j = min + float(j)*f
			ed_pal(i+1,z) = tlut(j)
		Enddo
	Enddo

	Call edit2lut(luts,vol*2+1,1,256)

	Return
	End
C
	Subroutine	edit2lut(luts,state,st,en)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'pals.inc'

	Integer*4	i,luts(3,256,4),state,st,en,p1,p2
	Real*4		r,g,b,a,h,s,v
C
C	convert from the current colormode and copy into luts
C
	If (en .lt. st) Then
		p1 = en
		p2 = st
	Else
		p1 = st
		p2 = en
	Endif
	Do i=p1,p2
		r = ed_pal(i,1)
		g = ed_pal(i,2)
		b = ed_pal(i,3)
		a = ed_pal(i,4)
		If (color_model .eq. 2) Then
			Call HSV_to_RGB(%val(r),%val(g),%val(b),h,s,v)
			luts(state,i,1) = h*255.0
			luts(state,i,2) = s*255.0
			luts(state,i,3) = v*255.0
			luts(state,i,4) = a*255.0
		Else
			luts(state,i,1) = r*255.0
			luts(state,i,2) = g*255.0
			luts(state,i,3) = b*255.0
			luts(state,i,4) = a*255.0
		Endif
	Enddo
C
	Return
	End
C
C	Routine to implement most of the one-shot tools
C
	Subroutine	click_tool(it,luts,edit,state,change)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'pals.inc'

	Integer*4	it,luts(3,256,4),state,edit(3,4)
	Integer*4	i,j,r,l,d,change,k,steps,z
	Real*4		f,acc,inc,tvec(256)

	change = 0
	If (it .eq. 1) Then ! Copy
		j = 1
		Do i=pins(1)+1,pins(2)+1
			cbuffer(j,1) = luts(state,i,1)
			cbuffer(j,2) = luts(state,i,2)
			cbuffer(j,3) = luts(state,i,3)
			cbuffer(j,4) = luts(state,i,4)
			j = j + 1
		Enddo
		ccount = j-1

	Else if (it .eq. 2) Then ! Paste
		j = 1
		Do i=pins(1)+1,pins(2)+1
			if(edit(state,1).eq.1)luts(state,i,1)=cbuffer(j,1)
			if(edit(state,2).eq.1)luts(state,i,2)=cbuffer(j,2)
			if(edit(state,3).eq.1)luts(state,i,3)=cbuffer(j,3)
			if(edit(state,4).eq.1)luts(state,i,4)=cbuffer(j,4)
			j = j + 1
			if (j .gt. ccount) goto 10
		Enddo
10		Continue
		Call lut2edit(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 3) Then ! M-Paste
		j = 1
		Do i=pins(1)+1,pins(2)+1
			if(edit(state,1).eq.1)luts(state,i,1)=cbuffer(j,1)
			if(edit(state,2).eq.1)luts(state,i,2)=cbuffer(j,2)
			if(edit(state,3).eq.1)luts(state,i,3)=cbuffer(j,3)
			if(edit(state,4).eq.1)luts(state,i,4)=cbuffer(j,4)
			j = j + 1
			if (j .gt. ccount) j = 1
		Enddo
		Call lut2edit(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 4) Then ! Reset
		Do i=pins(1)+1,pins(2)+1
			if(edit(state,1).eq.1)luts(state,i,1)=i-1
			if(edit(state,2).eq.1)luts(state,i,2)=i-1
			if(edit(state,3).eq.1)luts(state,i,3)=i-1
			if(edit(state,4).eq.1)luts(state,i,4)=i-1
		Enddo
		Call lut2edit(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 5) Then ! Linear
		if (pins(1) .eq. pins(2)) return
		Do i=1,4
			if (edit(state,i) .eq. 1) Then
				acc = ed_pal(pins(1)+1,i)
			inc = (ed_pal(pins(1)+1,i)-ed_pal(pins(2)+1,i))/
     +				(pins(1)-pins(2))
				Do j=pins(1),pins(2)
					ed_pal(j+1,i) = acc
					acc = acc + inc
				Enddo
			Endif
		Enddo
		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 6) Then ! Smooth
		if (pins(1) .eq. pins(2)) return
		Do i=1,4
			if (edit(state,i) .eq. 1) Then
				Do j=1,256
					tvec(j) = ed_pal(j,i)
				Enddo
				Do j=pins(1),pins(2)
					f=tvec(j+1)*1.0
					if ((j+1).gt. 255) then
						f = f + tvec(j+1)
					Else
						f = f + tvec(j+1+1)
					Endif
					if ((j-1).lt.0) then
						f = f + tvec(j+1)
					Else
						f = f + tvec(j-1+1)
					Endif
					ed_pal(j+1,i) = f/3.0
				Enddo
			Endif
		Enddo
		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 7) Then ! VFlip
		l = pins(1)
		r = pins(2)
		d = ((r-l)/2)+1
		Do j=l,r
			Do i=1,4
				if(edit(state,i).eq.1) Then
     				ed_pal(j+1,i)=1.0-ed_pal(j+1,i)
				Endif
			Enddo
		Enddo
		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if (it .eq. 8) Then ! HFlip
		l = pins(1)
		r = pins(2)
		d = ((r-l)/2)+1
		Do j=l,(r-d)
			Do i=1,4
				if(edit(state,i).eq.1) Then
				f=ed_pal(r-(j-l)+1,i)
				ed_pal(r-(j-l)+1,i) = ed_pal(j+1,i)
				ed_pal(j+1,i) = f
				Endif
			Enddo
		Enddo
		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if ((it .gt. 1000).and.(it .lt. 1256)) Then ! posterize

		steps = it-1000
		f = float(pins(2)-pins(1))/float(steps)
		if (f .le. 0) return
		Do z=1,4
			if (edit(state,z) .eq. 1) Then
				acc = float(pins(1))+f
				i = pins(1)
				Do j=0,steps-1
					d = i
					Do while (i .le. acc)
					   ed_pal(i+1,z)=ed_pal(d+1,z)
					   i = i + 1
					Enddo
					acc = acc + f
				Enddo
			Endif
		Enddo
		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Else if ((it .ge. 2001).and.(it .le. 2007)) Then ! bitslice

		steps = it-2000
		Do z=1,4
			if (edit(state,z) .eq. 1) Then
				Do j=1,256
					tvec(j) = ed_pal(j,z)
				Enddo
				Do j=0,255
					If ((j .ge. pins(1)).and.
     +						(j .le. pins(2))) Then
					    i = iand((2**(8-steps))-1,j)
					    i = i * (2**steps)
C
C	Should pick the center of the bucket???
C					    i = i + ((2**steps)-1)/2
C	For now take the bucket left edge (to copy protected whites...)
C
					    ed_pal(j+1,z)=tvec(i+1)
					Endif
				Enddo
			Endif
		Enddo

		Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		change = 1

	Endif

	Return
	End
C
C	Palette tools
C
	Subroutine	do_tool(state,luts,edit,disp,dialog,hist,csum)
	
	Implicit None	

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'pals.inc'

	Integer*4	state,luts(3,256,4),edit(3,4),disp(3,4)
	Integer*4	hist(256),csum
	
	Record /ditem/  dialog(*)

	Real*4	tbuff(256,4),c,wi,d
	Integer*4	oox,ooy
	Integer*4	leftx
	parameter	(leftx = 220)
	
	common/tool_buff/oox,ooy,tbuff,c,wi,d

	Integer*4	ox,oy,x,y,tag,status
	Integer*4	wleft,wbot
C
C	get the window position
C
	Call getori(wleft,wbot)
	ox=-999
	oy=-999
C
C	get the position
C
	X = getval(MOUSEX)-wleft-(leftx+1)
	Y = getval(MOUSEY)-wbot-6
C
C	is it viable??
C
	if ((x.lt.0).or.(y.lt.0).or.(x.gt.255).or.(y.gt.255)) Return
C
	Do while (getbut(MOUSE3))
C
C	get the position
C
		X = getval(MOUSEX)-wleft-(leftx+1)
		Y = getval(MOUSEY)-wbot-6		
		tag = 0
		if (x .ne. ox) tag = 1
		if (y .ne. oy) tag = 1
 		if (tag .eq. 1) Then
 			If (tool .eq. 1) Then
 				Call freehand_tool(ox,x,oy,y,state,luts,edit)
 			Else if (tool .eq. 2) Then
  				Call fiddle_tool(ox,x,oy,y,state,luts,edit)
			Else if (tool .eq. 3) Then
 				Call rotate_tool(ox,x,oy,y,state,luts,edit)
 			Endif
			Call DRAW_DLG(dialog,status)
			Call draw_hist(hist,csum)
			Call draw_luts(state,disp,edit,luts)
			Call swapbu
 		Endif
        	ox = x
        	oy = y
	Enddo
		
	Return
	End
C
C Freehand palette tool
C
	Subroutine	freehand_tool(ox,x,oy,y,state,luts,edit)
		
	Implicit None
		
	Include 'pals.inc'

	Integer*4	state,luts(3,256,4),edit(3,4)

	Integer*4	ox,x,oy,y
	Integer*4	i,j,k

	Real*4	tbuff(256,4),c,wi,d
	Integer*4	oox,ooy
	
	common/tool_buff/oox,ooy,tbuff,c,wi,d
C
C	Bracket the settings
C
	j = x
	Call clipit(j,pins(1),pins(2))
	if (ox .eq. -999) Then
		k = j
	Else
		k = ox
	Endif
	Call clipit(k,pins(1),pins(2))
	d = float(y)/255
	if (d .gt. 1.0) d = 1.0
	if (d .lt. 0.0) d = 0.0
C
C	set value
C
	Do i=1,4
		if (edit(state,i) .eq. 1) ed_pal(j+1,i) = d
	Enddo
C
C	Smooth out
C
	Call spread(j+1,k+1,state,edit)
	Call edit2lut(luts,state,j+1,k+1)

	Return
	End
C
C Fiddle (brightness/contrast) palette tool
C
	Subroutine	fiddle_tool(ox,x,oy,y,state,luts,edit)
		
	Implicit None
		
	Include 'pals.inc'

	Integer*4	state,luts(3,256,4),edit(3,4)

	Integer*4	i,j,k
	Integer*4	ox,x,oy,y
	Real*4	tbuff(256,4),c,wi,d,z
	Integer*4	oox,ooy
	
	common/tool_buff/oox,ooy,tbuff,c,wi,d
		
	If (ox .eq. -999) Then
		oox = x
		ooy = y
		c = 0.0
		wi = 1.0
		d = float(pins(2)-pins(1))/2.0
		Do i=1,256
			tbuff(i,1) = ed_pal(i,1)
			tbuff(i,2) = ed_pal(i,2)
			tbuff(i,3) = ed_pal(i,3)
			tbuff(i,4) = ed_pal(i,4)
		Enddo
		Return
	Endif
	
	c = -(x-oox)
	wi = (float(ooy-y)/256.0)+1.0
	if (wi .gt. 3.0) wi = 3.0
	if (wi .lt. 0.001) wi = 0.001
	Do k=1,4
		If (edit(state,k) .eq. 1) Then
			Do i=pins(1),pins(2)
				z = i-(pins(1)+d)+c
				z = z/d
				z = (z*d)/wi
				z = z + (pins(1)+d)
				j = int(z)
				if (j .lt. pins(1)) j = pins(1)
				if (j .gt. pins(2)) j = pins(2)
				ed_pal(i+1,k) = tbuff(j+1,k)
			Enddo
		Endif
	Enddo
	Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		
	Return
	End
C
C	Rotational Palette tool
C
	Subroutine	rotate_tool(ox,x,oy,y,state,luts,edit)
		
	Implicit None
		
	Include 'pals.inc'

	Integer*4	state,luts(3,256,4),edit(3,4)
	Integer*4	ox,x,oy,y

	Integer*4	i,j,k,step
	Real*4	tbuff(256,4),c,wi,d
	Integer*4	oox,ooy
	
	common/tool_buff/oox,ooy,tbuff,c,wi,d
		
	If (ox .eq. -999) Then
		oox = x
		ooy = y
		Do i=1,256
			tbuff(i,1) = ed_pal(i,1)
			tbuff(i,2) = ed_pal(i,2)
			tbuff(i,3) = ed_pal(i,3)
			tbuff(i,4) = ed_pal(i,4)
		Enddo
		Return
	Endif
		
	step = (oox - x)
	Do k=1,4
		if (edit(state,k) .eq. 1) Then
			Do i=pins(1),pins(2)
				j = i + step
				Do while(j .gt. pins(2)) 
					j=j-(pins(2) - pins(1) + 1)
				Enddo
				Do while(j .lt. pins(1)) 
					j=j+(pins(2) - pins(1) + 1)
				Enddo
				ed_pal(i+1,k) = tbuff(j+1,k)
			Enddo
		Endif
	Enddo
	Call edit2lut(luts,state,pins(1)+1,pins(2)+1)
		
	Return
	End
C
C	Clipping function
C
	Subroutine	clipit(val,low,high)
	
	Implicit None
	
	Integer*4	val,low,high
	
	if (val .gt. high) val = high
	if (val .lt. low) val = low
	
	Return
	End

