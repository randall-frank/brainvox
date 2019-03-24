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
C    MODULE NAME    : autotrace    
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : ?????
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 03 Aug 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to start finding a trace...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C	13 Nov 89 : rjf
C			converted from a BT subroutine to a general purpose
C			image tracing routine...
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call autotrace(image,size,level,trace,num,iin,jin)
C    INPUTS         : 
C			image : array of bytes assumed to be size*size to
C				be leveled
C			size : length of a scan line in bytes
C			level : byte value to contour at
C			iin,jin : place to start on the grid
C    OUTPUTS        : trace : output xy coords
C		      num : number of xy coords (returns 0 on error)
C
C    LIMITATIONS    :      
C    NOTES          :      This routine slides left from the i,j start
C				looking for a contour level and then
C				follows the contour BETWEEN the pixels
C				The returned trace is CLOSED (that is the
C				last point is a duplicate of the first)
C                                                                               
C    ====================================================================== 
	Subroutine autotrace(image,size,dose,trace,Iin,Jin)

	Implicit None

	Integer*4	Trace(4096,2),dose
	Integer*2	image(*)  
	Integer*4	size          
	Integer*4	num,I,J,Iin,Jin
C
	Integer*4	stx,sty,dir,sdir
	Integer*4	p(4),K
	Integer*4	v(4)
	Integer*4	d(2,2,2)
C
C	image access function allowing access to unsigned bytes
C
	Integer*4	Grid
C
	external	grid
C
	d(1,1,1)= +1
	d(1,1,2)=  0
	d(1,2,1)= -1
	d(1,2,2)= -1
	d(2,1,1)= -1
	d(2,1,2)= +1
	d(2,2,1)=  0
	d(2,2,2)= +1
C
C	locations.   3   4    directions.  1
C					 4-+-2
C		     1   2                 3
C	cases:
C              -   -          - | +
C	1,1,1    +--    1,1,2   | 
C        +1    - | +     +0   - | +
C
C              +   -          +   +
C	1,2,1  --+      1,2,2 --+ 
C        -1    - | +     -1   - | +
C
C              -   -          -   +
C	2,1,1  --+      2,1,2   +--
C        -1    + | -     +1   + | -
C
C              + | -          +   +
C	2,2,1    |      2,2,2   +--
C        +0    + | -     +1   + | -
C
C	If the point pair specified by (I,J)-(I+1,J) dose not bracket DOSE
C	the routine is to return NUM=0 (trace(1,1) = 0)
C
	trace(1,1) = 0
  	num=1  ! always start at +1
	I=Iin
	J=Jin
C
C	check for point in image...
C
988	If ((I.lt.2).or.(I.gt.size-1).or.(J.lt.2).or.(J.gt.size-1)) then
		Return
	Endif
C
C	test to see if we even start... (if the dose is equal consider it
C	  to be greater than dose)
C	If both are greater or less then slide to the left...
C
	If ((grid(I,J,size,image) .lt. dose).and.
     +		(grid(I+1,J,size,image) .lt. dose)) Then   
			i=i-1
			goto 988
	Endif
	If ((grid(I,J,size,image) .ge. dose).and.
     +		(grid(I+1,J,size,image) .ge. dose)) then
			i=i-1
			goto 988
	Endif
C
C	set the starting location and begin...
C
C	directions...                     1
C					4-+-2
C					  3
C
	Stx=I
	Sty=J
	Sdir=1
C
C	choose to start upward...
C
	dir=1
C
C	start the loop...                       v3   v4
C	Pick the values and evaluate them...       ^   
C                                               v1 | v2
100	If (dir .eq. 1) then
		v(1)=grid(I,J,size,image)
		v(2)=grid(I+1,J,size,image)
		v(3)=grid(I,J+1,size,image)
		v(4)=grid(I+1,J+1,size,image)
	Elseif (dir .eq. 2) then
		v(1)=grid(I,J+1,size,image)
		v(2)=grid(I,J,size,image)
		v(3)=grid(I+1,J+1,size,image)
		v(4)=grid(I+1,J,size,image)
	Elseif (dir .eq. 3) then
		v(1)=grid(I+1,J+1,size,image)
		v(2)=grid(I,J+1,size,image)
		v(3)=grid(I+1,J,size,image)
		v(4)=grid(I,J,size,image)
	Elseif (dir .eq. 4) then
		v(1)=grid(I+1,J,size,image)
		v(2)=grid(I+1,J+1,size,image)
		v(3)=grid(I,J,size,image)
		v(4)=grid(I,J+1,size,image)
	Endif
C
C	compare v1,v2,v3,v4
C
	Do K=1,4
		If (v(k) .lt. dose) then
			p(k)=1
		Else
			p(k)=2
		Endif
	Enddo
C
C	add the point between v(1) and v(2) to the list...
C
	num=num+1
C
C	if trace is big then close and return
C
C	If (num .gt. 4094) then
C		Trace(num,1)=Trace(2,1)
C		Trace(num,2)=Trace(2,2)
C		Trace(1,1) = num - 1
C		Return
C	Endif
C
C	if trace is big then thin and continue
C
	if (num .ge. 4094) then
		trace(1,1) = num - 1
		Call tr_thinroi(trace)
		num = trace(1,1) + 1
		write(*, 998)
998	Format("brainvox_trace - Warning, thining ROI during autotrace.")
	Endif
C
C	set the value..Based on direction (Left hand corner for P(1)=2 
C		or right hand corner for p(1)=1)
C
	If (dir .eq. 1) then
	    If (p(1) .eq. 2) then
		Trace(num,1)=(I)
		Trace(num,2)=(J)
	    Else
		Trace(num,1)=(I+1)
		Trace(num,2)=(J)
	    Endif
	Elseif (dir .eq. 2) then
	    If (p(1) .eq. 2) then
		Trace(num,1)=(I)
		Trace(num,2)=(J+1)      
	    Else
		Trace(num,1)=(I)
		Trace(num,2)=(J)      
	    Endif
	Elseif (dir .eq. 3) then
	    If (p(1) .eq. 2) then
		Trace(num,1)=(I+1)
		Trace(num,2)=(J+1)
	    Else
		Trace(num,1)=(I)
		Trace(num,2)=(J+1)
	    Endif
	Elseif (dir .eq. 4) then
	    If (p(1) .eq. 2) then
		Trace(num,1)=(I+1)
		Trace(num,2)=(J)
	    Else
		Trace(num,1)=(I+1)
		Trace(num,2)=(J+1)
	    Endif
	Endif
	Trace(num,1)=Trace(num,1)+0
	Trace(num,2)=Trace(num,2)+0
C
C	If the point is a repeat then do not add it....
C
	If (num .gt. 2) then
		If ((trace(num,1) .eq. trace(num-1,1)) .and.
     +			(trace(num,2) .eq. trace(num-1,2))) num=num-1
	Endif
C
C	Move along according to the table...Move only into a wall
C	where one corner is + and the other -
C	Note that by definition p(2)=3-p(1) and is not used...
C
	dir=dir+d(p(1),p(3),p(4))
C
C	bracket...
C
	If (dir .ge. 5) dir=1
	If (dir .le. 0) dir=4
C
C	bump I and J along...
C
	If (dir .eq. 1) then
		J=J+1
	Elseif (dir .eq. 2) then
		I=I+1
	Elseif (dir .eq. 3) then
		J=J-1
	Elseif (dir .eq. 4) then
		I=I-1
	Endif
C
C	test for stop criteria...
C
	If ((dir .eq. sdir).and.(I .eq. Stx).and.(J .eq. Sty)) then
C
C	replicate first point and return
C
		num=num+1
		Trace(num,1)=Trace(2,1)
		Trace(num,2)=Trace(2,2)
		Trace(1,1) = Num - 1
		Return
C
	Endif
C
	goto 100
C
	End
C
C----------------------------------------------------------------
C	Local function to provide access to the image in I*4 form
C----------------------------------------------------------------
C
	Integer*4	Function	grid(i,j,size,image)

	Implicit None
	
	Integer*4	I,J,size
	Integer*2	image(*)
C
C	Check for offscreen 
C	Handle this by returning an outrageously large number...
C
	If ((I.lt.1).or.(I.gt.size).or.(J.lt.1).or.(J.gt.size)) then
		grid=4096   !pretty outrageous???
		Return
	Endif
C
C	get the value and return
C
	grid=image((j-1)*size+I)
C
C	fix from BYTE conversion  (unused because of avail I*2 data)
C
C	If (grid .lt. 0) grid=256+grid  
C
	grid = grid - 256   ! remove the offset added for LUT display
C
	Return
	End
