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
C    MODULE NAME    : split_hemis
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
C    DATE           : 3 Oct 1995      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to split a brain into 2 hemispheres using
C			the optimal graph search routine.
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
	Subroutine split_hemis(mode)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'  
	Include 'tr_inc.f'  
C
	Integer*4	whole_rnum,lhemi_rnum,rhemi_rnum,i,j
	Integer*4	depth(512*512),mode,slice
	Integer*4	saved_cslice,midline(512)
	Integer*4	it,ib,y,jr,jl,it_i,ib_i,sxr,sxl
C
C	Check for valid roi names
C
	whole_rnum = 0
	lhemi_rnum = 0
	rhemi_rnum = 0
	Do i=1,numrgns
	if (index(rgnnames(i),"whole.roi") .ne. 0) whole_rnum = i
	if (index(rgnnames(i),"lefthemi.roi") .ne. 0) lhemi_rnum = i
	if (index(rgnnames(i),"righthemi.roi") .ne. 0) rhemi_rnum = i
	Enddo
C
	if ((whole_rnum*lhemi_rnum*rhemi_rnum) .eq. 0) Then
		Call bv_error("One of the required rois|"//
     +			"whole.roi,righthemi.roi,lefthemi.roi|"//
     +			"could not be found.}",0)
		Return
	Endif
C
C	"mask" the volume by adding 1 to the texture volume
C
	write(*,*)"Masking the texture volume..."
	Call do_texture_op(IPC_TEX_CL_INCR,1,whole_rnum)
C
C	Add blurred texture and MRI volume (where the texture volume is > 1)
C	To create the cost matrix
C
	Call hemi_search(%val(imagesize),%val(imagesize),%val(realimages),
     +		%val(interpimages),
     +		%val(texture_addr),%val(images_addr),depth)
C
	if (mode .eq. 0) return
C
C	Create the ROIs and cut to subdirs
C	Depth = (realimages x imagesize)
C
	saved_cslice = cur_slice
	Do slice=1,realimages
C
		Call draw_status_bar(float(slice)/float(realimages),
     +			"Cutting...}")
C
C	read the whole ROIs
C	rois(num,4096,2) rois(num,1,1) = numpoints
C
		cur_slice = slice
		Call tr_read_roi(whole_rnum)
C
C	Get the midline buffer (note: y is stored upside down?)
C
		Do i=0,imagesize-1
			midline(imagesize-i)=
     +				depth(slice+(i*realimages))+128-20
		Enddo
C
C	Find the whole-midline intersections (two of them)
C
		it = -1
		ib = -1
		Do i = 2,rois(whole_rnum,1,1)+1
			y = rois(whole_rnum,i,2)/2
			if (y .lt. 1) y = 1
			if (y .gt. 256) y = 256
C
C	found an intersection?
C
			If (abs(midline(y) - (rois(whole_rnum,i,1)/2)) 
     +				.lt. 2) Then
				If ((y .gt. it).or.(it .eq. -1)) Then
					it = y
					it_i = i
				Endif
				If ((y .lt. ib).or.(ib .eq. -1)) Then
					ib = y
					ib_i = i
				Endif
			Endif
		Enddo
C
C	Second pass, allow (ib) to rise and (it) to drop as long as
C	the difference in index values is not more than 1/10 the total number
C
		If ((it .ne. -1).and.(ib .ne. -1)) Then
		jr = it_i
		jl = ib_i
		Do i = 2,rois(whole_rnum,1,1)+1
			y = rois(whole_rnum,i,2)/2
			if (y .lt. 1) y = 1
			if (y .gt. 256) y = 256
C
C	found an intersection?
C
			If (abs(midline(y) - (rois(whole_rnum,i,1)/2)) 
     +				.lt. 2) Then
				If (y .lt. it) Then
	j = abs(i - jr)
	If (j .gt. rois(whole_rnum,1,1)/2) Then
		j = abs(j - rois(whole_rnum,1,1))
	Endif
					If (j .lt. 
     +					    rois(whole_rnum,1,1)/10) Then
						it = y
						it_i = i
					Endif
				Endif
				If (y .gt. ib) Then
	j = abs(i - jl)
	If (j .gt. rois(whole_rnum,1,1)/2) Then
		j = abs(j - rois(whole_rnum,1,1))
	Endif
					If (j .lt. 
     +					    rois(whole_rnum,1,1)/10) Then
						ib = y
						ib_i = i
					Endif
				Endif
			Endif
		Enddo
		Endif
C
C	Did we find two intersections???
C
		If ((it .ne. -1).and.(ib .ne. -1)) Then
C
C	Piece the ROIs together
C	start with the midline
C
			jr = 0
			jl = 0
			sxr = 0
			sxl = 0
			Do i=ib+1,it-1
				jr = jr + 1
				jl = jl + 1
				rois(rhemi_rnum,jr+1,1)=midline(i)*2+1
				rois(rhemi_rnum,jr+1,2)=i*2
				rois(lhemi_rnum,jl+1,1)=midline(i)*2+1
				rois(lhemi_rnum,jl+1,2)=i*2
				sxr = sxr + rois(rhemi_rnum,jr+1,1)
				sxl = sxl + rois(lhemi_rnum,jl+1,1)
			Enddo
C
C	take off right
C
			i = it_i
			Do while(i .ne. ib_i) 
				i = i + 1
				if (i .gt. rois(whole_rnum,1,1)+1) i = 2
				jr = jr + 1
				rois(rhemi_rnum,jr+1,1)=rois(whole_rnum,i,1)
				rois(rhemi_rnum,jr+1,2)=rois(whole_rnum,i,2)
				sxr = sxr + rois(rhemi_rnum,jr+1,1)
			Enddo
			sxr = sxr / float(jr)
C
C	take off left
C
			i = it_i
			Do while(i .ne. ib_i) 
				i = i - 1
				if (i .lt. 2) i = rois(whole_rnum,1,1)+1
				jl = jl + 1
				rois(lhemi_rnum,jl+1,1)=rois(whole_rnum,i,1)
				rois(lhemi_rnum,jl+1,2)=rois(whole_rnum,i,2)
				sxl = sxl + rois(lhemi_rnum,jl+1,1)
			Enddo
			sxl = sxl / float(jl)
C
			rois(rhemi_rnum,1,1) = jr
			rois(lhemi_rnum,1,1) = jl
C
C	swap them if needed 
C
			If (sxl .lt. sxr) Then
			    Do i=1,4096
				it = rois(rhemi_rnum,i,1)
				ib = rois(rhemi_rnum,i,2)
				rois(rhemi_rnum,i,1)=rois(lhemi_rnum,i,1)
				rois(rhemi_rnum,i,2)=rois(lhemi_rnum,i,2)
				rois(lhemi_rnum,i,1) = it
				rois(lhemi_rnum,i,2) = ib
			    Enddo
			Endif
		Else
C
C	Copy the whole roi?
C
			Do i=2,rois(whole_rnum,1,1)+1
			    rois(rhemi_rnum,i,1) = rois(whole_rnum,i,1)
			    rois(lhemi_rnum,i,1) = rois(whole_rnum,i,1)
			    rois(rhemi_rnum,i,2) = rois(whole_rnum,i,2)
			    rois(lhemi_rnum,i,2) = rois(whole_rnum,i,2)
			Enddo
			rois(rhemi_rnum,1,1) = rois(whole_rnum,1,1)
			rois(lhemi_rnum,1,1) = rois(whole_rnum,1,1)
		Endif
C
C	store the results (and cutout the images) 
C
		Call tr_write_roi(rhemi_rnum)
		Call tr_write_roi(lhemi_rnum)

	Enddo
C
C	restore things for the user...
C
	cur_slice = saved_cslice
	Call tr_read_roi(whole_rnum)
	Call tr_read_roi(rhemi_rnum)
	Call tr_read_roi(lhemi_rnum)
	
	Return
	End
