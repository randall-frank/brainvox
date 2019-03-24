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
C    MODULE NAME    : polyfill   
C    VERSION        : 1.1.1
C    LOCATION       :  
C    LAST EDIT      :  13 Jul 95
C    STATUS         :  (Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        :   General purpose filling routine
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 14 Nov 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine will compute the locations of the
C			points in the interior of a polygon.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C	14 Nov 89	rjf
C			converted from Pascal & pseudocode from Computer
C			Graphics book into FORTRAN77.
C
C	07 Feb 91	rjf
C			Fixed bug in sort_on_bigger_y which caused the
C			termination criteria to be wrong.
C
C       13 Jul 95       rjf
C                       Fixed bug which assumed (side_count) was > 0.
C                       (Consider all points on a horiz line!!)
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :    Call polyfill(trace,count,list,ptr) 
C    INPUTS         :    Trace(4096,2) contains x,y points defining the poly
C			     this poly must be closed!!!
C			 Count is the number of points in the Trace array      
C			 PTR is the number of I*4 elements in LIST
C    OUTPUTS        :    PTR is the number of integers used in LIST.
C			 LIST contains X1,X2,Y triples defining lines within
C			 	the polygon.
C    LIMITATIONS    :      
C    NOTES          :      If PTR returns equal to -1 the LIST array was
C			   too small to hold all the returned line segments.
C                 
C    ====================================================================== 
	Subroutine  polyfill(trace,count,list,ptr)

	Implicit None

	Integer*4	trace(4096,2),count    ! the x,y coords of the poly
	Integer*4	list(*),ptr  !x1,x2,y of interior points.

	Integer*4	Max_list
C
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure /each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record /each_entry/	sides(0:max_points)
C
	Integer*4	side_count,first_s,last_s
	Integer*4	scan,bottomscan,x_int_count,r
C
C	On the way in PTR is the size of the LIST array... Elements of
C	LIST will be in the form LIST(X)=X1 LIST(X+1)=X2 LIST(X+2)=Y
C	Which represents a line of pixels from X1,Y to X2,Y   PTR is the
C	length to which LIST is filled on exit.  (i.e. LIST(PTR) is the Y
C	value of the last line)  If PTR=-1 on exit then there was not enough
C	space in the array to hold the lines...
C
	Max_list=Ptr
	Ptr=0
C
C	Handle a trivial case...
C
	If (Count .le. 2) Return
C
C	On to the real routine...
C
	Call sort_on_bigger_y(count,trace,sides,side_count,bottomscan)
C
	first_s=1
	last_s=1
C
C	Cannot assume there are ANY sides!!
C
	If (side_count .eq. 0) Return
C
	Do scan=sides(1).y_top,bottomscan,-1
		Call update_first_last(sides,count,scan,first_s,last_s)
		Call process_x_intersections(sides,scan,first_s,last_s,
     +				x_int_count)
		Call draw_lines(sides,scan,x_int_count,first_s,
     +				list,ptr,max_list)
		Call update_sides_list(sides,first_s,last_s)
	Enddo
C
	Return
	End
C
C----------------------level 2 -------------------------------
C
	Subroutine	sort_on_bigger_y(n,trace,sides,side_count,
     +			bottomscan)

	Implicit None

	Integer*4	N,Trace(4096,2)
	Integer*4	side_count,bottomscan
C
	Integer*4	Max_points,mnext_y
	parameter (Max_points=4096)
	external	mnext_y
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Integer*4	k,x1,y1
C
	side_count=0
	y1=trace(n,2)
	x1=trace(n,1)
	bottomscan=y1
C	
	Do K=1,n
		If (y1 .ne. trace(k,2)) then
			side_count=side_count+1
			Call put_in_sides_list(sides,side_count,x1,y1,
     +				trace(k,1),trace(k,2),
     +				mnext_y(trace,n,k))
		Else
C
C	Horizontal line
C
		Endif
C
C	Patched 7 feb 91:rjf
C		bottomscan stuff was inside the else clause
C		now it is where it should be.
C
		if (trace(k,2) .lt. bottomscan) then
			bottomscan=trace(k,2)
		Endif
C
		y1=trace(k,2)
		x1=trace(k,1)
C		
	Enddo
C
	Return
	End
C
C----------------------level 3 -------------------------------
C
	Integer*4	function mnext_y(trace,n,k)

	Implicit None
	
	Integer*4	trace(4096,2),n,k,j
C
	mnext_y	= trace(k,2)
C
	j=k
100	If (trace(j,2) .ne. mnext_y) then
		mnext_y=trace(j,2)
		return
	Endif
	j=j+1
	If (j .le. n) goto 100
	j=1
	goto 100
C
	End
C
C----------------------level 3 -------------------------------
C
	Subroutine put_in_sides_list(sides,ientry,ix1,iy1,ix2,iy2,next_y)

	Implicit None

	Integer*4	entry,x1,x2,y1,y2,next_y,ientry
	Integer*4	ix1,ix2,iy1,iy2
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Integer*4	maxy
	Real*4		x2_temp,x_change_temp
C
	entry=ientry
	x1=ix1
	x2=ix2
	y1=iy1
	y2=iy2
C
	x_change_temp=float(x2-x1)/float(y2-y1)
	x2_temp=float(x2)
C
	If ((y2 .gt. y1).and.(y2 .lt. next_y)) then
		y2=y2-1
		x2_temp=x2_temp-x_change_temp
	Elseif ((y2 .lt. y1).and.(y2 .gt. next_y)) then
		y2=y2+1
		x2_temp=x2_temp+x_change_temp
	Endif
C
	If (y1 .gt. y2) then
		maxy=y1
	Else
		maxy=y2
	Endif
C
	do while ((Entry .gt. 1).and.(maxy .gt. sides(entry-1).y_top))
		sides(entry).y_top=sides(entry-1).y_top
		sides(entry).x_int=sides(entry-1).x_int
		sides(entry).delta_y=sides(entry-1).delta_y
		sides(entry).x_change_per_scan=
     +			sides(entry-1).x_change_per_scan
		entry=entry-1
	Enddo
C
C	with (sides(entry))...
C
	sides(entry).y_top=maxy
	sides(entry).delta_y=abs(y2-y1)+1
	If (y1 .gt. y2) then	
		sides(entry).x_int=x1
	Else
		sides(entry).x_int=x2_temp
	Endif
	sides(entry).x_change_per_scan=x_change_temp
C
C	end {with}
C
	return
	End
C
C
C----------------------level 2 -------------------------------
C
	Subroutine update_first_last(sides,count,scan,first_s,last_s)
C
	Implicit None

	Integer*4	count,scan,first_s,last_s
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Do while((sides(last_s+1).y_top .ge. scan).and.
     +			(last_s .lt. count))
		last_s=last_s+1
	Enddo
C
	Do while (sides(first_s).delta_y .eq. 0)
		first_s=first_s+1
	Enddo
C
	Return
	End
C
C----------------------level 2 -------------------------------
C
	Subroutine process_x_intersections(sides,scan,first_s,last_s,
     +				x_int_count)
C
	Implicit None

	Integer*4	x_int_count,scan,first_s,last_s
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Integer*4	k
C
	x_int_count=0
C
	Do k=first_s,last_s
		If (sides(k).delta_y .gt. 0) then
			x_int_count=x_int_count+1
			Call sort_on_x(k,first_s,sides)
		Endif
	Enddo
C
	Return
	End
C
C----------------------level 3 -------------------------------
C
	Subroutine sort_on_x(ientry,first_s,sides)

	Implicit None

	Integer*4	entry,first_s,ientry
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
	Record/each_entry/	temprec
C
	entry=ientry
C
	do while ((entry .gt. first_s).and.
     +		(sides(entry).x_int .lt. sides(entry-1).x_int))
C
C	swap(sides(entry),sides(entry-1))
C	again heavy use of aggregate assignment statments...
C
		temprec.y_top=sides(entry).y_top
		temprec.x_int=sides(entry).x_int
		temprec.delta_y=sides(entry).delta_y
		temprec.x_change_per_scan=
     +			sides(entry).x_change_per_scan

		sides(entry).y_top=sides(entry-1).y_top
		sides(entry).x_int=sides(entry-1).x_int
		sides(entry).delta_y=sides(entry-1).delta_y
		sides(entry).x_change_per_scan=
     +			sides(entry-1).x_change_per_scan

		sides(entry-1).y_top=temprec.y_top
		sides(entry-1).x_int=temprec.x_int
		sides(entry-1).delta_y=temprec.delta_y
		sides(entry-1).x_change_per_scan=
     +			temprec.x_change_per_scan
C
		entry=entry-1
	Enddo
C
	Return
	End
C
C----------------------level 2 -------------------------------
C
	Subroutine draw_lines(sides,scan,x_int_count,iindex,
     +			list,ptr,max_list)
C
	Implicit None
C
	Integer*4	list(*),ptr,max_list
	Integer*4	scan,x_int_count,index,iindex
	Integer*4	Max_points
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Integer*4	k,x,x1,x2
C
	index=iindex
C
	Do K=1,jnint(float(x_int_count)/2.0)
C
		Do while (sides(index).delta_y .eq. 0)
			index=index+1
		Enddo
C
		x1=jnint(sides(index).x_int)
		index=index+1
C
		Do while (sides(index).delta_y .eq. 0)
			index=index+1
		Enddo
C
		x2=jnint(sides(index).x_int)
C
C	place the line in the buffer if there is room.
C	If the buffer fills then return PTR=-1.
C
		If (ptr .ne. -1) then
			ptr=ptr+3
			If (ptr .le. max_list) then
				list(ptr)=scan
				list(ptr-1)=x2
				list(ptr-2)=x1  
			Else
				ptr=-1
			Endif
		Endif
C
C	next line segment.
C
		index=index+1
	Enddo
C
	Return
	End
C
C----------------------level 2 -------------------------------
C
	Subroutine update_sides_list(sides,first_s,last_s)
C
	Implicit None
C
	Integer*4	Max_points,first_s,last_s
	parameter (Max_points=4096)
C
	Structure/each_entry/
		Integer*4 	y_top
		Real*4	 	x_int
		Integer*4	delta_y
		Real*4		x_change_per_scan
	End Structure
C
	Record/each_entry/	sides(0:max_points)
C
	Integer*4	k
C
	Do K=first_s,last_s
C
C	do with sides(k)
C
		If (sides(k).delta_y .gt. 0) then
			sides(k).delta_y=sides(k).delta_y-1
			sides(k).x_int=sides(k).x_int-
     +				sides(k).x_change_per_scan
		Endif
C
C	enddo {with}
C
	Enddo
C
	Return
	End
