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
C    MODULE NAME    : closehemi
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
C    DATE           : 15 Jan 91       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	
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
	Subroutine closehemi(trace,heminum,error)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	trace(4096,2)
	Integer*4	hemi(4096,2)
	Integer*4	I,p1,p2,j,step,heminum
	Real*4		d1,d2,t
	logical		error
C
C	assume error condition
C
	error = .true.
C
C	if trace is already closed return
C
	I = trace(1,1)+1
        If ((abs(trace(2,1)-trace(I,1)) .lt. 2) .and.
     +          (abs(trace(2,2)-trace(I,2)) .lt. 2)) return
C
C	if trace is very small return
C
	if (trace(1,1) .lt. 4) return
C
C	this routine closes a trace (trace) my finding its intersection with
C	the HEMISPHERE  (rois(heminum,...)
C
        Do I=1,rois(heminum,1,1)+1
                hemi(I,1)=rois(heminum,I,1)
                hemi(I,2)=rois(heminum,I,2)
        Enddo
C
C	if hemi is small or undef return
C
	if (hemi(1,1) .lt. 4) return
C
C	j is index of last trace point
C
	j = trace(1,1) + 1
C
C	find indexes
C
	p1 = -1
	p2 = -1
	d1 = 9999999.0
	d2 = 9999999.0
C
C	look along hemi
C
	Do I=2,hemi(1,1)+1
		t  = (float(trace(2,1) - hemi(I,1))**2.0) +
     +			 (float(trace(2,2) - hemi(I,2))**2.0)	
		If (t .lt. d1) then
			d1 = t
			p1 = I
		Endif
		t  = (float(trace(j,1) - hemi(I,1))**2.0) +
     +			 (float(trace(j,2) - hemi(I,2))**2.0)	
		If (t .lt. d2) then
			d2 = t
			p2 = I
		Endif
	Enddo
C
C	Find which direction to go... (p2->p1)
C
	i = p2
	j = 0
	Do while (p1 .ne. i)
		j = j + 1
		i = i + 1
		If (i .gt. hemi(1,1)+1) I = 2
	Enddo
C
C	Now add on the hemi stretch to the old roi
C
	If (J .lt. (hemi(1,1)/2)) then
		step = 1
	Else
		step = -1
	Endif
C
C	Now short sect is from p2 to p1 (or p1 to p2)
C
	I = p2
	J = trace(1,1) + 1
	Do while (p1 .ne. i)
		If (j .ge. 4094) Then
			trace(1,1) = j
			write(*, *)"Warning, ROI too long.  Please thin it."
			return
		Endif
		j = j + 1
		trace(j,1) = hemi(i,1)
		trace(j,2) = hemi(i,2)
		i = i + step
		if (i .lt. 2) i = hemi(1,1) + 1
		If (i .gt. hemi(1,1)+1) I = 2
	Enddo
	trace(j+1,1) = trace(2,1)
	trace(j+1,2) = trace(2,2)
	trace(1,1) = j 
C
	error = .false.
C
	Return
	End
