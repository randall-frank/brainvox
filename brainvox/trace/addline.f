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
C    MODULE NAME    : addline
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
C    DESCRIPTION    :  	This routine adds a line segment to the end of
C			a trace.  The segment is specified by a single point
C			and the segment is sampled to the pixel level.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call addline(trace,x,y)
C    INPUTS         :          x,y point to be added to the trace..
C    OUTPUTS        : 		trace : the resultant ROI
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine addline(trace,x,y)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  
C
	Integer*4	x,y,trace(4096,2)
C
	Integer*4	ox,oy,I,J
	Real*4		dx,dy,len
C
C	x and y are in image coordinates...!!!
C	(0-512) (0-512)
C
C	start a new trace if there are no points...
C
	If (trace(1,1) .eq. 0) then
		trace(2,1)=x
		trace(2,2)=y
		trace(1,1)=1
		return
	Endif
C
	If (trace(1,1) .ge. 4090) then
	write(*, *)"Warning, ROI too long.  Please thin before you continue."
		return
	Endif
C
C	remember that 1,1 is the number of points in trace.
C	The last point is (1,1)+1
C
	I=trace(1,1)+1
C
C	Find the origin point...
C
	ox=trace(I,1)
	oy=trace(I,2)
C
C	duplicate point???
C
	If ((ox .eq. x).and.(oy .eq. y)) return
C
C	draw a line from oldx,oldy to x,y
C
	dx=float(x-ox)
	dy=float(y-oy)
C	
	len=sqrt(dx**2+dy**2)
	dx=dx/len
	dy=dy/len
C
	j=1
C
C	while the segment does not end at the input point
C	fill in points...
C
	Do while ((abs(x-trace(I,1)) .gt. 1) .or. 
     +			(abs(y-trace(I,2)) .gt. 1))
		trace(I+1,1)=ox+jifix(dx*float(J))
		trace(I+1,2)=oy+jifix(dy*float(J))
		If ((trace(I,1) .ne. trace(I+1,1)).or.
     +			(trace(I,2) .ne. trace(I+1,2))) then
			I=I+1
			If (I .ge. 4090) Then
	write(*, *)"Warning, ROI too long.  Please thin before you continue."
				goto 999
			Endif
		Endif
		j=j+1
	Enddo
C
C	store off endpoint
C
999	I=I+1
	trace(I,1)=x
	trace(I,2)=y
C
C	done...
C
	trace(1,1)=i-1
C
C	error if not enough room to add points..
C
	Return
	End
