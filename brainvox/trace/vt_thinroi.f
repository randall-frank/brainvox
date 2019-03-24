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
C    MODULE NAME    : tr_thinroi   
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
C    DATE           : 05 Nov 90       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to reduce the number of points in a trace
C			by averaging.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call tr_thinroi(trace)
C    INPUTS         :          trace(4096,2) : brainvox form ROI
C    OUTPUTS        : 	       trace is modified and returned
C
C    LIMITATIONS    :      
C    NOTES          :      This is the brainvox format ROI (1,1)=numpts
C			and this routine halves the number of input pts
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  tr_thinroi(trace)

	Implicit None

	Integer*4	trace(4096,2),I,J,temp(4096,2)
	logical		closed
C
C	if few points then return
C	
	if (trace(1,1) .lt. 8) return
C
C	is the input ROI closed ?
C
	j = trace(1,1)+1
	if ((trace(2,1) .eq. trace(j,1)) 
     +		.and. (trace(2,2) .eq. trace(j,2))) then
		closed = .true.
	Else
		closed = .false.
	Endif
C
C	average each pair of points together
C
	j = 0
	Do I=3,trace(1,1)+1,2
		j = j + 1
		temp(j,1) = (trace(i,1) + trace(i-1,1))/2
		temp(j,2) = (trace(i,2) + trace(i-1,2))/2
	Enddo
C
C	copy the new roi back for return
C
	trace(1,1) = j
	Do I=1,j
		trace(i+1,1) = temp(i,1)
		trace(i+1,2) = temp(i,2)
	Enddo
C
C	if old roi was closed then make sure this one is also
C
	if ((closed) .and. 
     +		((trace(2,1) .ne. trace(j+1,1)) .or.
     +		 (trace(2,2) .ne. trace(j+1,2)) ) ) then
		trace(1,1) = j + 1
		trace(j+2,1) = trace(2,1)
		trace(j+2,2) = trace(2,2)
	Endif
	
	Return
	End
