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
C    MODULE NAME    : doedit.f
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
C    DESCRIPTION    :  	This routine removes a portion of a trace defined
C			by three points.  The last point becomes the new
C			end point.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call doedit(trace)
C    INPUTS         :          trace : trace to have section removed
C    OUTPUTS        : 	       trace : modified
C
C    LIMITATIONS    :      
C    NOTES          :      uses common block /global/ to pass points
C                 
C                                                                               
C    ====================================================================== 
	Subroutine  doedit(trace)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	trace(4096,2),I,pt(3),j,k,tr(4096,2),first
	real*4		dist(3),d
C
C	get end of the list...
C
	I=trace(1,1)+1
C
C	find the closest points... (assume not even close)
C	edit(point#,xy) holds the three points
C
	dist(1)=1000.0
	dist(2)=1000.0
	dist(3)=1000.0
C
C	all points in the trace
C
	Do J=2,I
C
C	all three edit points
C
		Do K=1,3
			d=sqrt(float(trace(J,1)-edit(K,1))**2 +
     +				float(trace(J,2)-edit(K,2))**2)
C
C	if new d is closer then select it...
C
			If (d .lt. dist(K)) then
				pt(k)=j
				dist(k)=d
			Endif
		Enddo
	Enddo
C
C	Now start at point 2 and walk along until you reach point 1 or 3
C
	J=pt(2)
100	J=J+1
C
C	wrap around...
C
	If (J .gt. I) J=2
C
C	is it 1 or 3 ???
C
	If ((J .ne. pt(1)).and.(J .ne. pt(3))) goto 100
C                                
	first=1
	if (j .eq. pt(3)) first=3
C
C	now record points until you reach the other point
C
        K=1
    	tr(k,1)=trace(J,1)
	tr(k,2)=trace(J,2)
200	j=j+1
        if (j .gt. I) J=2
	If ((J .eq. pt(1)).or.(J .eq. pt(3))) goto 300
C
C	record the point in the tr array
C
        K=k+1
    	tr(k,1)=trace(J,1)
	tr(k,2)=trace(J,2)
        goto 200
C
C	record the last point...
C
300    	k=k+1
   	tr(k,1)=trace(J,1)
	tr(k,2)=trace(J,2)              
C
C	now string out the tr array into the TRACE array
C	if the first point was 1 then do it forward
C	else do it in reverse order this leaves the last point
C	clicked on as the end of the trace.  The user need only continue
C	tracing instead of moving to the first point and trace...
C
	If (first .eq. 1) then
		Do J=1,K           
C
C	remember to leave room for 1,1
C
			trace(j+1,1)=tr(J,1)
			trace(j+1,2)=tr(J,2)
		Enddo
		trace(1,1)=k
	Else                   
                I=1         
C
C	make room for trace(1,1)...
C
		Do J=K,1,-1               
			I=I+1
			trace(I,1)=tr(J,1)
			trace(I,2)=tr(J,2)
		Enddo                       
		trace(1,1)=I-1
	Endif
C
	Return
	End
