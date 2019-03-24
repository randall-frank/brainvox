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
C    MODULE NAME    : tr_read_roi_disk
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
C    DESCRIPTION    :  	This routine reads an ROI from disk...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call tr_read_roi_disk(num)
C    INPUTS         :          Num : roi number to read
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_read_roi_disk(num)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	
	Integer*4	num,I,J,out_slice,strlen
	Character*200	filename,temp,roi,dir
	logical		found,error
	External strlen
C
C	read the roi named by CUR_SLICE and ROINAMES(NUM)
C
	rois(num,1,1)=0  ! just in case
C
C	build the filename
C
        temp=''
	Call get_slice_num(cur_slice,out_slice)
        Call Name_Changer(rgnnames(num),out_slice,temp,error)
        If (error) then
                write(*, 998)out_slice,rgnnames(num)
998     Format(' brainvox_trace - Error: unable to read slice '
     +		,I3,' using:',A40)
                goto 999
        Endif
C
C	prefix the filename with appropriate directory
C
	if (cut_dir  .eq. 0) then
		filename=temp(1:strlen(temp))
	Else
		Call indexnum_to_roi(cut_dir,roi,i)
		Call roi_to_dir(roi,dir)
		filename=dir(1:strlen(dir))//"/"//
     +			temp(1:strlen(temp))
C
C	special case for hemispheres
C
		If (num .eq. 1) then
			filename=temp(1:strlen(temp))
		Endif
	Endif
C
C	is it there??? (assume the worst...)
C
	inquire(file=filename,exist=found)
	If (.not. found) return
C
	open(66,file=filename,form='formatted',status='old',err=999,readonly)
C
C	read number of points..
C
	read(66,*,err=997,end=997) rois(num,1,1)
C
C	half the number of points is the number of coords...
C
	rois(num,1,1)=rois(num,1,1)/2
	If (rois(num,1,1) .gt. 4094) Then
		rois(num,1,1) = 4094
                write(*,996)filename
996     Format("Brainvox_trace - Warning:Truncated ROI. "//
     +		"Too many points:",A)
	Endif
C
	Do I=2,rois(num,1,1)+1
		read(66,*,err=997,end=997) rois(num,I,1)
		read(66,*,err=997,end=997) rois(num,I,2)
	Enddo
C
	Close(66)
C
	Return
C
997	rois(num,1,1) = 0
	Close(66)
	Return
C
999	rois(num,1,1)=0
	Return
	End
