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
C    MODULE NAME    : tr_write_roi
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
C    DESCRIPTION    :  	This routine writes an ROI to disk...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call tr_write_roi(num)
C    INPUTS         :          Num : roinunmber to write...
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_write_roi(num)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	
	Integer*4	num,I,J,trace(4096,2),statb(12),out_slice
	character*100	filename,temp,roi,dir
	logical		found,error

	integer*4	bv_access,strlen
	external	bv_access,strlen
C
C	save to dynamic memory...
C
	trace(1,1)=rois(num,1,1)
	Do I=2,rois(num,1,1)+1
		trace(I,1)=rois(num,I,1)
		trace(I,2)=rois(num,I,2)
	Enddo
	Call savetrace(cur_slice,num,trace)
C
C	write the roi named by CUR_SLICE and ROINAMES(NUM)
C	build filename
C
	temp=''
	Call get_slice_num(cur_slice,out_slice)
 	Call Name_Changer(rgnnames(num),out_slice,temp,Error)
	If (error) then
		write(*, 998)out_slice,rgnnames(num)
998	Format(' brainvox_trace - Error: unable to save slice ',
     +		I3,' using:',A40)
		goto 999
	Endif
C
C       prefix the filename with appropriate directory
C
        if (cut_dir  .eq. 0) then
                filename=temp(1:strlen(temp))
	Else
		Call indexnum_to_roi(cut_dir,roi,i)
		Call roi_to_dir(roi,dir)
		filename=dir(1:strlen(dir))//"/"//
     +			temp(1:strlen(temp))
C
C       special case for hemispheres
C
        	If (num .eq. 1) then
                	filename=temp(1:strlen(temp))
        	Endif
        Endif
C
C	is it there??? 
C
	inquire(file=filename,exist=found)
C
	If (found) Then
		i = bv_access(filename(1:strlen(filename))//char(0),"w")
		If (i .ne. 0) goto 999
	Endif

C
C	select the proper open...
C
	If (.not. found) then  
C
		open(66,file=filename,form='formatted',status='new',err=999)
C
	Else
C
		open(66,file=filename,form='formatted',status='old',err=999)
C
	endif
C
C	write the ROI to disk...
C
15	Format(I5)
	write(66,15,err=996) rois(num,1,1)*2
C
	Do I=2,rois(num,1,1)+1
		write(66,15,err=996) rois(num,I,1)
		write(66,15,err=996) rois(num,I,2)
	Enddo
C
	Close(66,err=999)
C
C	if it is a hemisphere then cut the image out
C
	if (cut_dir  .eq. 0) then
		Call cutout(trace,num,error)
	Endif
C
	Return
C
996	Close(66,err=999)
C
999	write(*, 997)
997	Format("brainvox - Unable to save ROI to disk. ",/,
     +	       "           Check disk space and write permission.")

        Call bv_error("Unable to save ROI to disk|"//
     +                  "Check disk space and permissions.}",0)

	Call ringbe

	Return

	End
