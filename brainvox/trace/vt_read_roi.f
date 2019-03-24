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
C    MODULE NAME    : tr_read_roi
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
C    DESCRIPTION    :  	This routine reads an roi from dynamic memory
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call tr_read_roi(num)
C    INPUTS         :          num : roinmuber to read...
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_read_roi(num)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
	
	Integer*4	num,I,trace(4096,2)
C
C	read the roi named by CUR_SLICE and ROINAMES(NUM)
C	get it from dynamic memory...
C
	Call gettrace(cur_slice,num,trace)
C
C	copy the retrieved image into the ROIS list
C
	rois(num,1,1)=trace(1,1)
	Do I=2,rois(num,1,1)+1
		rois(num,I,1)=trace(I,1)
		rois(num,I,2)=trace(I,2)
	Enddo
C
	Return
C
	End
