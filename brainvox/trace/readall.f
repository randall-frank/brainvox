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
C    MODULE NAME    : readall 
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
C    DESCRIPTION    :  	This routine reads in all of the images and the
C			rois into dynamic memory.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call Readall
C    INPUTS         :          All via common
C    OUTPUTS        : 	       All via common
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  readall

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	Include 'vt_arb_inc.f'

	Integer*4	fid,status,I,K,temp(4096,2),J
	Integer*4	slices,t_cur_slice
	Byte		image(512*512)
	Character*200	filename
C
	slices = realimages
	If (arb_mode .ne. 0) Then
		slices = cur_settings(arb_mode).num_slices
	Endif
C
C	Only load any new slices needed
C
	If (oldslices .ge. slices) Return
C
	t_cur_slice = cur_slice
C
C	but up a bar graph...
C
	call draw_status_bar(float(0)/float(slices),
     +			"Loading regions...}")
C
C	Read in all the rois... (Images are provided via shared memory)
C
	Do I=oldslices+1,slices 
C
C	Read the ROIs... (Only ODD slice numbers if skip=2)
C
		If (((iand(I,1) .eq. 1).and.(skip .eq.2))
     +				.or.(skip.eq.1)) then
C
			cur_slice=I
C
			Do J=1,numrgns
C
C	read it in
C
				Call tr_read_roi_disk(J)
C
C	Copy to temp
C
				temp(1,1)=rois(J,1,1)
				Do K=2,rois(J,1,1)+1
					temp(K,1)=rois(J,K,1)
					temp(K,2)=rois(J,K,2)
				Enddo
C
C	copy temp to dynamic memory
C
				Call savetrace(I,J,temp)
C
			Enddo
C
		Endif
C
C	update slider...
C
		call draw_status_bar(float(i)/float(slices),
     +			"Loading regions...}")
C
	Enddo
C
	oldslices = slices
	cur_slice = t_cur_slice
C	
	Return
	End
