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
C    MODULE NAME    : paintpoints
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to draw point lists onto images
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
	Subroutine  paintpoints(num,list,slice,zoom,offset,myset)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
 	Include 'windows.inc'

	Integer*4	num,list(4096,4),slice,zoom,offset(2)
	Integer*4	I,p(4),j,slicetoZ,error,out(4),usergb
	Integer*4       zbuff_ptr,image_ptr,zdx,zdy,myset(*)
	Integer*4	clip
	External	Slicetoz
	Real*4		temp
C
	Integer*4	FAT,clippingon,tal_over,clipshade
C
        common  /fat_com/fat,clippingon,tal_over,clipshade
C
C
C	color list
C
	Integer*4	col(28)

	data 	col/'00000000'X,
     +		    '000000ff'X,
     +		    '0000ff00'X,
     +		    '0000ffff'X,
     +		    '00ff0000'X,
     +		    '00ff00ff'X,
     +		    '00ffff00'X,
     +		    '00ffffff'X,
     +              '00007fff'X,
     +              '0000ff7f'X,
     +              '007fff00'X,
     +              '00ff7f00'X,
     +              '00ff007f'X,
     +              '007f00ff'X,
     +	            '00000000'X,
     +		    '000000bf'X,
     +		    '0000bf00'X,
     +		    '0000bfbf'X,
     +		    '00bf0000'X,
     +		    '00bf00bf'X,
     +		    '00bfbf00'X,
     +		    '00bfbfbf'X,
     +              '00005fbf'X,
     +              '0000bf5f'X,
     +              '005fbf00'X,
     +              '00bf5f00'X,
     +              '00bf005f'X,
     +              '005f00bf'X/
C
C	check error space
C
	temp = float(slicetoz(2) - slicetoz(1))/2.0
	if (temp .gt. 1.7) temp = 1.7
C
	If (slice .eq. -1) Then
C
C	Get the clipping buffer
C
		Call vl_getbufs(image_ptr,zbuff_ptr,zdx,zdy,myset)
	Endif
C
C	Get visual mode
C
        If ((getdis() .eq. DMRGB).or.(getdis() .eq. DMRGBD)) then
		usergb = 1
        Else
		usergb = 0
        Endif
C
C	run thru list
C
	Do I=1,num
C
		if (slice .eq. -1) then
C
C	if 3d points then RGBmode
C
			p(1) = (list(I,1)-offset(1))*zoom
			p(2) = (list(I,2)-offset(2))*zoom
			p(3) = p(1) + (zoom - 1) +FAT
			p(4) = p(2) + (zoom - 1) +FAT
			Call clip_to_zbuffer(list(I,1),list(I,2),
     +			   list(I,3),%val(zbuff_ptr),zdx,zdy,clip,1)
C
			j = list(I,4)+1
C
C	"dim" points behind if not clipped...
C

			if ((clip.ne.0).and.(clipshade.ne.0)) Then
				j = j + 14 
C	Treat "dimmed" points as unclipped...
				clip = 0 
			Endif
			If (usergb .eq. 1) Then
				Call cpack(col(j))
			Else
				Call set_color_index(col(j))
			Endif
			if (clip .eq. 0) Then
				Call sboxfi(p(1),p(2),p(3),p(4))
			Else
				If (clippingon .eq. 0) Then
					Call sboxfi(p(1),p(2),p(3),p(4))
				Endif
			Endif
C
		Else
C
C	if 2d points then cmode
C
C	Are we in cuttingplane mode?
C
			If (wind2d(curwin).mode .eq. 0) then
C
C	Is the point on the slice?
C
				If (abs(slicetoz(slice)-list(I,3)) 
     +						.le. temp) then
C
			If (usergb .eq. 1) Then
				Call cpack(col(list(I,4)+1))
			Else
				Call set_color_index(col(list(I,4)+1))
			Endif
					p(1) = (list(I,1)-offset(1))*zoom
					p(2) = (list(I,2)-offset(2))*zoom
					p(3) = p(1) + (zoom - 1) +FAT
					p(4) = p(2) + (zoom - 1) +FAT
					Call sboxfi(p(1),p(2),p(3),p(4))
C
				Endif
			Else 
				p(1) = list(I,1)
				p(2) = list(I,2)
				p(3) = list(I,3)
				Call vl_3dtocutplane(p(1),out(1),temp,error)
C DEBUG
C	type *,"Paintpoints:in",p(1),p(2),p(3),"out:",out(1),out(2),out(3)
				If (error .eq. 0) then
			If (usergb .eq. 1) Then
				Call cpack(col(list(I,4)+1))
			Else
				Call set_color_index(col(list(I,4)+1))
			Endif
					p(1) = (out(1)-offset(1))*zoom
					p(2) = (out(2)-offset(2))*zoom
					p(3) = p(1) + (zoom - 1) +FAT
					p(4) = p(2) + (zoom - 1) +FAT
					Call sboxfi(p(1),p(2),p(3),p(4))
				Endif

			Endif
		Endif
C
	Enddo
C
	Return
	End


	Integer*4 Function ZtoSlice(Z)

	Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include 'voxeluif.inc'
	
	Integer*4	Z,P

        p = int(((float(Z))/float(interpimages+1))+1.49)
        if (p .lt. 1) p = 1
        if (p .gt. realimages) p = realimages

	Ztoslice = p

	return
	End

	Integer*4 Function SlicetoZ(slice)

	Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include 'voxeluif.inc'
	
	Integer*4	slice

	slicetoz = (slice + (slice-1)*interpimages) -1

	return
	End

	Integer*4 Function ZtoSlice_t(Z,t)

	Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include 'voxeluif.inc'
	
	Integer*4       slicetoz,ztoslice
	External        slicetoz,ztoslice

	Integer*4	sfromz,t,zfroms,Z

	sfromz = ZtoSlice(Z)
	zfroms = SlicetoZ(sfromz)
	
	If (abs(zfroms-Z) .le. t) Then
		Ztoslice_t = sfromz
	Else
		Ztoslice_t = -1
	Endif

	return
	End
C
C	Routine to clip points to the Zbuffer
C	returns 1 if clipping should occur 0 otherwise
C
	Subroutine clip_to_zbuffer(x,y,z,zbuffer,dx,dy,clip,flag)

	Implicit None

	Integer*2       zbuffer(*)
	Integer*4       dx,dy,x,y,z
	Integer*4	clip,flag
	Real*4		zp

	Real*4		fudge
	parameter	(fudge = 3.3)

	clip = 0
	If (flag .eq. 0) return  ! default is no clipping

	zp = float(zbuffer(y*dx+x+1))/8.0

	If ((float(z)+FUDGE) .lt. zp) clip = 1

C	type *,'zbuffer=',zp,' x,y,z*8=',x,y,z+1

	Return
	End
