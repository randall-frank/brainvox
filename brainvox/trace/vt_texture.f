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
C    MODULE NAME    : vt_texture
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
C    DATE           : 3 Aug 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines for modifying the texture volume via ROI
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
	Subroutine do_texture_op(op,value,roi)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'tr_inc.f'  
	Include 'vt_arb_inc.f'

	Integer*4	op,value,roi

	Integer*4       thintrace(4096,2)
	Real*4          fact,d,dist
	Integer*4	count,list(4096),ptr,interp
	Integer*4	trace(4096,2),i,x,y,j,k,slice,n_slices
	Integer*4	v(3),pi(3),arb_mode_save
	Integer*4	xstart,xend,clip

	Real*4		p(3),xa(3),ya(3),za(3),hd(3),squ
	Real*4		v2(3),v3(3)
C
	Byte		buffer(512*512)
C
C	Get number of slices
C
	n_slices = realimages
	If (arb_mode .ne. 0) Then
C
C	Save off temp arb mode
C
		arb_mode_save = arb_mode
C
C	See if a secondary arb_mode would be preferred
C
		Call roiname_to_arbmode(rgnnames(roi),arb_mode)
		Call Switch_Arb_Mode
C
		n_slices = cur_settings(arb_mode).num_slices
C
C	Get the arb settings (slice 1)
C
		Call slice_to_helddata(1)
		Call vl_get_held_data(p,xa,ya,za,hd,squ)
		Do i=1,3
			if (abs(xa(i)) .lt. 1e-10) xa(i) = 0.0
			if (abs(ya(i)) .lt. 1e-10) ya(i) = 0.0
			if (abs(za(i)) .lt. 1e-10) za(i) = 0.0
		Enddo
C
C	One slice thickness (ip spacing)
C
		dist = (cur_settings(arb_mode).islice)/	(interpixel)
	Endif
C
C	over all slices
C
        Call draw_status_bar(float(0)/float(n_slices),
     +                  "Working...}")
C
	Do slice=1,n_slices
C
C	Get the ROI (roi) and divide by value
C
		Call gettrace(slice,roi,trace)
	        fact = 512.0/float(imagesize)
C
C       convert the input trace format to the polyfill trace format
C       Note: the ROI must be shrunk by FACT to go from 512x512 to imagesize
C
        	count = 0          
        	Do i=1,trace(1,1)
                	X = (float(trace(i+1,1))/fact)
                	Y = (float(trace(i+1,2))/fact)
                	if (X .lt. 0) X = 0
               	 	If (Y .lt. 0) Y = 0
                	if (X .gt. imagesize-1) X = imagesize - 1
                	If (Y .gt. imagesize-1) Y = imagesize - 1
C
C       thin out replicants
C
                	if (count .eq. 0) then         ! always the first one
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Else if (X .ne. thintrace(count,1)) then 
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Else if (Y .ne. thintrace(count,2)) then 
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Endif
C
        	Enddo
C
C	Perform the filling operation
C
        	ptr=4096
        	Call polyfill(thintrace,count,list,ptr)
C		write(*,*)slice,roi,trace(1,1),count,ptr,op,value,texture_addr
        	If (ptr .gt. 1) then
			If (arb_mode .ne. 0) Then
C
C	Make the mask slice
C
				Do i=1,imagesize*imagesize
					buffer(i) = 0
				Enddo
				Do i=1,ptr-2,3
                			Y=list(i+2)
                			Do X=list(i),list(i+1)
						Do j=list(i)+(y*imagesize),
     +						   list(i+1)+(y*imagesize)
							buffer(j+1) = 1
						Enddo
					Enddo
				Enddo
C
C	Over all of the volume
C
				Do k=0,(hd(3)*2)-1
					v(3) = k
				Do j=0,(hd(2)*2)-1
					v(2) = j
C
C	Compute xstart and xend values (speeding things up)
C
				xstart = 0
				xend = (hd(1)*2)-1
C
C	Compute plane equation (pxnx+pyny+pznz=d)
C
				v2(1) = p(1) + (dist*(slice-1)*za(1))
				v2(2) = p(2) + (dist*(slice-1)*za(2))
				v2(3) = p(3) + (dist*(slice-1)*za(3))
C
				d = v2(1)*za(1)+v2(2)*za(2)+v2(3)*za(3)
C
C	Two cases: parallel to x scanline or not...
C
				If (za(1) .eq. 0) Then
C
C	Is the point (any x value) within +-dist from the plane?
C
					v3(1) = (0.0-hd(1)) - v2(1)
					v3(2) = (v(2)-hd(2)) - v2(2)
					v3(3) = (v(3)-hd(3))*squ - v2(3)
					d = v3(1)*za(1) + v3(2)*za(2) +
     +						v3(3)*za(3)
C
C	Clip it out if too far away
C
					If (abs(d) .gt. abs(dist)) Then
						xstart=1
						xend=0
					Endif
				Else
C
C	intersect x scanline and the plane (get x location)
C
					v2(2) = v(2) - hd(2)
					v2(3) = (v(3) - hd(3))*squ
					v2(1) = (d-v2(2)*za(2)-v2(3)*za(3))
     +							/za(1)
C
C	clip bounds = (clip)(Nx)=(dist/2):(clip=(dist/Nx)for safety)
C
					clip = min(abs(dist*0.75/za(1)),
     +							10000.)
C	X location back to voxel coords
					d = v2(1) + hd(1)
                                        d = min(d,10000.)
C
C	and clip to +-dist from the plane
C
					If ((d - clip) .gt. xstart) Then
						xstart = d - clip
					Endif
					If ((d + clip) .lt. xend) Then
						xend = d + clip
					Endif
				Endif
C
C	Do the x scanline
C
				Do i=xstart,xend
					v(1) = i
C
					v2(1) = v(1)-hd(1)
					v2(2) = v(2)-hd(2)
					v2(3) = (v(3)-hd(3))*squ
C
C	Vec from point to plane
C
					v3(1) = v2(1) - p(1)
					v3(2) = v2(2) - p(2)
					v3(3) = v2(3) - p(3)
C
C	Dot prod (distance to plane)
C
					d = v3(1)*za(1) + v3(2)*za(2) +
     +						v3(3)*za(3)
C
C	in range (map into slice number)
C
					d = (d/dist)+0.5
C
C	+1 from slice starts at 1, not 0 
C
					If ((slice .eq. int(d)+1).and.
     +						(d .gt. 0.0)) Then
C
C	map onto plane
C
						pi(1) = v3(1)*xa(1) +
     +							v3(2)*xa(2) +
     +							v3(3)*xa(3)
						pi(2) = v3(1)*ya(1) +
     +							v3(2)*ya(2) +
     +							v3(3)*ya(3)
						pi(1)=pi(1)+(imagesize/2)
						pi(2)=pi(2)+(imagesize/2)
C
		If ((pi(1) .ge. 0) .and. (pi(1).lt.imagesize)
     +			.and.(pi(2).ge.0).and.(pi(2).lt.imagesize) )Then
C
C	Perform operation on (v)
C
		If (buffer(pi(1)+(imagesize*pi(2))+1) .ne. 0) Then
      			Call volume_op(op,value,v,%val(texture_addr),
     +				imagesize,imagesize)
		Endif
C
		Endif
 
					Endif

				Enddo
				Enddo
				Enddo
C
			Else 
C
C	Modify the texture volume
C
				j = texture_addr+((slice-1)*imagesize
     +					*imagesize*(1+interpimages))
				Call texure_slice(op,value,list,ptr,%val(j),
     +					imagesize,imagesize)
C
C	Interpolate
C
				If (slice .ne. n_slices) Then
					Do interp=1,interpimages
						j=j+(imagesize*imagesize)
					Call texure_slice(op,value,list,ptr,
     +						%val(j),imagesize,imagesize)
					Enddo
				Endif
			Endif
		Endif
C
        	Call draw_status_bar(float(slice)/float(n_slices),
     +                  "Working...}")
	Enddo
C
C	Clean up (reset the held vectors and the incoming arb_mode)
C
	If (arb_mode .ne. 0) Then
C
		If (arb_mode .ne. arb_mode_save) Then
			arb_mode = arb_mode_save
			Call Switch_Arb_Mode
		Endif
		Call slice_to_helddata(cur_slice)
	Endif
	
	Return
	End
C
C	Perform the operation
C
	Subroutine texure_slice(op,value,list,ptr,data,dx,dy)

	Implicit None

	Integer*4	op,value,dx,dy
	Integer*4	i,j,x,y,jj
	Byte		data(*)
	Integer*4	ptr,list(*)

	Do i=1,ptr-2,3
                Y=list(i+2)
C		write(*, *)list(i),list(i+1),y,op,value
                Do X=list(i),list(i+1)

C
C       get the index and add to the count
C
                        jj=y*dx+x+1
C
C	Do the work
C
			j = data(jj)
			If (j .lt. 0) j=256+j
			Call texture_op(op,j,value)
			If (j .ge. 128) j=j-256
			data(jj) = j
		Enddo
	Enddo

	Return
	End
C
	Subroutine volume_op(op,value,v,data,dx,dy)

	Implicit None

	Integer*4	op,value,v(3),dx,dy
	Byte		data(*)

	Integer*4	j,jj

	jj = v(1)+(dx*v(2))+(dx*dy*v(3))+1

	j = data(jj)
	If (j .lt. 0) j=256+j
	Call texture_op(op,j,value)
	If (j .ge. 128) j=j-256
	data(jj) = j

	Return
	End
C
C	Do the texture operation
C
	Subroutine texture_op(op,j,value)

	Implicit None

	Include '../libs/vt_inc.f'  

	Integer*4	op,j,value

	If (op .eq. IPC_TEX_CL_PAINT) Then
		j = value
	Else if (op .eq. IPC_TEX_CL_TAG) Then
		j = ior(j,128)
	Else if (op .eq. IPC_TEX_CL_UNTAG) Then
		j = iand(j,127)
	Else if (op .eq. IPC_TEX_CL_INCR) Then
		j = j + 1
		if (j .ge. 255) j = 255
	Else if (op .eq. IPC_TEX_CL_DECR) Then
		j = j - 1
		if (j .le. 0) j = 0
	Endif

	Return
	End
C
C	Non-interactive version for the off line processing
C
	Subroutine offline_texture_op(op,value,roi)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  
	Include 'tr_inc.f'  
	Include 'vt_arb_inc.f'

	Integer*4	op,value
	Character*(*)	roi
	Character*256	filename
	logical		error

	Integer*4       thintrace(4096,2)
	Real*4          fact,d,dist
	Integer*4	count,list(4096),ptr,interp
	Integer*4	trace(4096,2),i,x,y,j,k,slice,n_slices
	Integer*4	v(3),pi(3),arb_mode_save
	Integer*4	xstart,xend,clip,strlen
	External	strlen

	Real*4		p(3),xa(3),ya(3),za(3),hd(3),squ
	Real*4		v2(3),v3(3)
C
	Byte		buffer(512*512)
C
C	Get number of slices
C
	n_slices = realimages
C
C	arb_mode flag should be correct
C
	If (arb_mode .ne. 0) Then
C
		n_slices = cur_settings(arb_mode).num_slices
C
C	Get the arb settings (slice 1)
C
		Call slice_to_helddata(1)
		Call vl_get_held_data(p,xa,ya,za,hd,squ)
C
C	One slice thickness (ip spacing)
C
		dist = (cur_settings(arb_mode).islice)/	(interpixel)
	Endif
C
	Do slice=1,n_slices
C
C	Get the ROI (roi) and divide by value
C
		Call name_changer(roi,slice,filename,error)
		write(*, *)"Reading ROI:",filename(1:strlen(filename))
		Call flush(6)
		Call read_roi_file(filename,trace,count,error)
		If (error) Then
			trace(1,1) = 0
		Else
			Do i=count,1,-1
				trace(i+1,1) = trace(i,1)
				trace(i+1,2) = trace(i,2)
			Enddo
			trace(1,1) = count
		Endif
		
	        fact = 512.0/float(imagesize)
C
C       convert the input trace format to the polyfill trace format
C       Note: the ROI must be shrunk by FACT to go from 512x512 to imagesize
C
        	count = 0          
        	Do i=1,trace(1,1)
                	X = (float(trace(i+1,1))/fact)
                	Y = (float(trace(i+1,2))/fact)
                	if (X .lt. 0) X = 0
               	 	If (Y .lt. 0) Y = 0
                	if (X .gt. imagesize-1) X = imagesize - 1
                	If (Y .gt. imagesize-1) Y = imagesize - 1
C
C       thin out replicants
C
                	if (count .eq. 0) then         ! always the first one
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Else if (X .ne. thintrace(count,1)) then 
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Else if (Y .ne. thintrace(count,2)) then 
                        	count = count + 1
                        	thintrace(count,1) = X
                        	thintrace(count,2) = Y
                	Endif
C
        	Enddo
C
C	Perform the filling operation
C
        	ptr=4096
        	Call polyfill(thintrace,count,list,ptr)
C		write(*,*)slice,roi,trace(1,1),count,ptr,op,value,texture_addr
        	If (ptr .gt. 1) then
			If (arb_mode .ne. 0) Then
C
C	Make the mask slice
C
				Do i=1,imagesize*imagesize
					buffer(i) = 0
				Enddo
				Do i=1,ptr-2,3
                			Y=list(i+2)
                			Do X=list(i),list(i+1)
						Do j=list(i)+(y*imagesize),
     +						   list(i+1)+(y*imagesize)
							buffer(j+1) = 1
						Enddo
					Enddo
				Enddo
C
C	Over all of the volume
C
				Do k=0,(hd(3)*2)-1
					v(3) = k
				Do j=0,(hd(2)*2)-1
					v(2) = j
C
C	Compute xstart and xend values (speeding things up)
C
				xstart = 0
				xend = (hd(1)*2)-1
C
C	Compute plane equation (pxnx+pyny+pznz=d)
C
				v2(1) = p(1) + (dist*(slice-1)*za(1))
				v2(2) = p(2) + (dist*(slice-1)*za(2))
				v2(3) = p(3) + (dist*(slice-1)*za(3))
C
				d = v2(1)*za(1)+v2(2)*za(2)+v2(3)*za(3)
C
C	Two cases: parallel to x scanline or not...
C
				If (za(1) .eq. 0) Then
C
C	Is the point (any x value) within +-dist from the plane?
C
					v3(1) = (0.0-hd(1)) - v2(1)
					v3(2) = (v(2)-hd(2)) - v2(2)
					v3(3) = (v(3)-hd(3))*squ - v2(3)
					d = v3(1)*za(1) + v3(2)*za(2) +
     +						v3(3)*za(3)
C
C	Clip it out if too far away
C
					If (abs(d) .gt. abs(dist)) Then
						xstart=1
						xend=0
					Endif
				Else
C
C	intersect x scanline and the plane (get x location)
C
					v2(2) = v(2) - hd(2)
					v2(3) = (v(3) - hd(3))*squ
					v2(1) = (d-v2(2)*za(2)-v2(3)*za(3))
     +							/za(1)
C
C	clip bounds = (clip)(Nx)=(dist/2):(clip=(dist/Nx)for safety)
C
					clip = min(abs(dist*0.75/za(1)),
     +							10000.)
C	X location back to voxel coords
					d = v2(1) + hd(1)
                                        d = min(d,10000.)
C
C	and clip to +-dist from the plane
C
					If ((d - clip) .gt. xstart) Then
						xstart = d - clip
					Endif
					If ((d + clip) .lt. xend) Then
						xend = d + clip
					Endif
				Endif
C
C	Do the x scanline
C
				Do i=xstart,xend
					v(1) = i
C
					v2(1) = v(1)-hd(1)
					v2(2) = v(2)-hd(2)
					v2(3) = (v(3)-hd(3))*squ
C
C	Vec from point to plane
C
					v3(1) = v2(1) - p(1)
					v3(2) = v2(2) - p(2)
					v3(3) = v2(3) - p(3)
C
C	Dot prod (distance to plane)
C
					d = v3(1)*za(1) + v3(2)*za(2) +
     +						v3(3)*za(3)
C
C	in range (map into slice number)
C
					d = (d/dist)+0.5
C
C	+1 from slice starts at 1, not 0 
C
					If ((slice .eq. int(d)+1).and.
     +						(d .gt. 0.0)) Then
C
C	map onto plane
C
						pi(1) = v3(1)*xa(1) +
     +							v3(2)*xa(2) +
     +							v3(3)*xa(3)
						pi(2) = v3(1)*ya(1) +
     +							v3(2)*ya(2) +
     +							v3(3)*ya(3)
						pi(1)=pi(1)+(imagesize/2)
						pi(2)=pi(2)+(imagesize/2)
C
		If ((pi(1) .ge. 0) .and. (pi(1).lt.imagesize)
     +			.and.(pi(2).ge.0).and.(pi(2).lt.imagesize) )Then
C
C	Perform operation on (v)
C
		If (buffer(pi(1)+(imagesize*pi(2))+1) .ne. 0) Then
      			Call volume_op(op,value,v,%val(texture_addr),
     +				imagesize,imagesize)
		Endif
C
		Endif
 
					Endif

				Enddo
				Enddo
				Enddo
C
			Else 
C
C	Modify the texture volume
C
				j = texture_addr+((slice-1)*imagesize
     +					*imagesize*(1+interpimages))
				Call texure_slice(op,value,list,ptr,%val(j),
     +					imagesize,imagesize)
C
C	Interpolate
C
				If (slice .ne. n_slices) Then
					Do interp=1,interpimages
						j=j+(imagesize*imagesize)
					Call texure_slice(op,value,list,ptr,
     +						%val(j),imagesize,imagesize)
					Enddo
				Endif
			Endif
		Endif
C
	Enddo

	Return
	End
