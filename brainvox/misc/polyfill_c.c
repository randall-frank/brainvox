#include "polyfill.h"

/*
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
C    VERSION        : 2.0.1
C    LOCATION       :  
C    LAST EDIT      : 13 Jul 95
C    STATUS         :  (Production,Obsolete)
C    LANGUAGE       : C
C    PACKAGE        :   General purpose polygon filling routine
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 10 Apr 92       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine will compute the locations of the
C			points in the interior of a polygon.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C	13 Jul 95	rjf
C			Fixed bug which assumed (side_count) was > 0.
C			(Consider all points on a horiz line!!)
C
C	10 Apr 92	rjf
C			Converted to C
C
C	07 Feb 91	rjf
C			Fixed bug in sort_on_bigger_y which caused the
C			termination criteria to be wrong.
C
C	14 Nov 89	rjf
C			converted from Pascal & pseudocode from Computer
C			Graphics book into FORTRAN77.
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :    Call polyfill(trace,count,list,ptr) 
C    INPUTS         :    Trace(8192) contains x,y points defining the poly
C			     this poly must be closed!!!
C			 Count is the number of elements in the Trace array
C				which equals the number of verticies*2      
C			 PTR is the number of I*4 elements in LIST
C    OUTPUTS        :    PTR is the number of integers used in LIST.
C			 LIST contains X1,X2,Y triples defining lines within
C			 	the polygon.
C    LIMITATIONS    :      
C    NOTES          :      If PTR returns equal to -1 the LIST array was
C			   too small to hold all the returned line segments.
C                 
C    ====================================================================== 
*/

#define	MAX_POINTS	8192
typedef struct {
	int 	y_top;
	double	x_int;
	int	delta_y;
	double	x_change_per_scan;
	} each_entry;

/* local prototypes */
int	mnext_y(int *trace, int count, int item);
void	sort_on_bigger_y(int n,int *trace,each_entry *sides,int *side_count,
			int *bottomscan);
void	put_in_sides_list(each_entry *sides,int ientry,int ix1,int iy1,
		int ix2,int iy2,int next_y);
void	update_first_last(each_entry *sides,int count,int scan,int *first_s,int *last_s);
void	process_x_intersections(each_entry *sides,int scan,int first_s,int last_s,
		int *x_int_count);
void 	sort_on_x(int ientry,int first_s,each_entry *sides);
void	draw_lines(each_entry *sides,int scan,int x_int_count,int iindex,
		int *list,int *ptr,int max_list);
void	update_sides_list(each_entry *sides,int first_s,int last_s);

void	polyfill_c_(int *trace,int count,int *list,int *ptr)
{
/*
	Integer*4	trace[8192],count    ! the x,y coords of the poly
			trace[0]=x1 trace[1]=y1 trace[2]=x2 trace[3]=y2 ...
	Integer*4	list[*],ptr  !x1,x2,y of interior points.
*/
	int		Max_list;

	each_entry	sides[MAX_POINTS];
	int		side_count,first_s,last_s;
	int		scan,bottomscan,x_int_count;
/*
C	On the way in PTR is the size of the LIST array... Elements of
C	LIST will be in the form LIST(X)=X1 LIST(X+1)=X2 LIST(X+2)=Y
C	Which represents a line of pixels from X1,Y to X2,Y   PTR is the
C	length to which LIST is filled on exit.  (i.e. LIST(PTR) is the Y
C	value of the last line)  If PTR=-1 on exit then there was not enough
C	space in the array to hold the lines...
*/
	Max_list=(*ptr);
	*ptr=0;

/*	Handle a trivial case... */
	if (count <= 2*2) return;
/*	On to the real routine... */

	sort_on_bigger_y(count,trace,sides,&side_count,&bottomscan);

	first_s=1;
	last_s=1;

/* We cannot assume ANY sides made it to the list!! */
	if (side_count == 0) return;

	for(scan=sides[1].y_top; scan >= bottomscan; scan = scan -1) {
			/* should this count be side_count??? */
		update_first_last(sides,side_count,scan,&first_s,&last_s);
		process_x_intersections(sides,scan,first_s,last_s,&x_int_count);
		draw_lines(sides,scan,x_int_count,first_s,list,ptr,Max_list);
		update_sides_list(sides,first_s,last_s);
	}

	return;
}

/*----------------------level 2 -------------------------------*/


void	sort_on_bigger_y(int n,int *trace,each_entry *sides,int *side_count,
			int *bottomscan)
{
	int	k,x1,y1;

	*side_count=0;
	y1=trace[n-1];
	x1=trace[n-2];
	*bottomscan=y1;
	
	for(k=0;k<n;k=k+2) {
		if (y1 != trace[k+1]) {
			*side_count=(*side_count)+1;
			put_in_sides_list(sides,*side_count,x1,y1,trace[k],
				trace[k+1],mnext_y(trace,n,k));
		} else {

/*	Horizontal line */

		}
/*
C	Patched 7 feb 91:rjf
C		bottomscan stuff was inside the else clause
C		now it is where it should be.
*/
		if (trace[k+1] < (*bottomscan)) {
			*bottomscan=trace[k+1];
		}

		y1=trace[k+1];
		x1=trace[k];
		
	}

	return;
}

/*----------------------level 3 -------------------------------*/

int	mnext_y(int *trace,int n,int k)
{
	int	j,out;

	out = trace[k+1];

	j=k;
l100:	if (trace[j+1] != out) {
		out = trace[j+1];
		return(out);
	}
	j = j + 2;
	if (j < n) goto l100;
	j=0;
	goto l100;
}

/*----------------------level 3 -------------------------------*/

void	put_in_sides_list(each_entry *sides,int ientry,int ix1,int iy1,
		int ix2,int iy2,int next_y)
{
	int	entry,x1,x2,y1,y2;
	int	maxy;
	double	x2_temp,x_change_temp;
 
	entry=ientry;
	x1=ix1;
	x2=ix2;
	y1=iy1;
	y2=iy2;

	x_change_temp=(double)(x2-x1)/(double)(y2-y1);
	x2_temp=(double)(x2);

	if ((y2 > y1) && (y2 < next_y)) {
		y2=y2-1;
		x2_temp=x2_temp-x_change_temp;
	} else if ((y2 < y1) && (y2 > next_y)) {
		y2=y2+1;
		x2_temp=x2_temp+x_change_temp;
	}

	if (y1 > y2) {
		maxy=y1;
	} else {
		maxy=y2;
	}

	while ((entry > 1) && (maxy > sides[entry-1].y_top)) {
		sides[entry] = sides[entry-1];
		entry=entry-1;
	}
/*	with (sides(entry))...*/
	sides[entry].y_top=maxy;
	sides[entry].delta_y=abs(y2-y1)+1;
	if (y1 > y2) {	
		sides[entry].x_int=x1;
	} else {
		sides[entry].x_int=x2_temp;
	}
	sides[entry].x_change_per_scan=x_change_temp;
/*	end {with} */
	return;
}

/*----------------------level 2 -------------------------------*/

void	update_first_last(each_entry *sides,int count,int scan,int *first_s,int *last_s)
{
	while((sides[(*last_s)+1].y_top >= scan) && ((*last_s) < count)) {
		*last_s=(*last_s)+1;
	}
 
	while (sides[(*first_s)].delta_y == 0) {
		*first_s=(*first_s)+1;
	}
	return;
}

/*----------------------level 2 -------------------------------*/

void	process_x_intersections(each_entry *sides,int scan,int first_s,
		int last_s,int *x_int_count)
{
	int k;
 
	*x_int_count=0;

	for(k=first_s; k<=last_s;k++) {
		if (sides[k].delta_y > 0) {
			*x_int_count=(*x_int_count)+1;
			sort_on_x(k,first_s,sides);
		}
	}

	return;
}

/*----------------------level 3 -------------------------------*/

void	sort_on_x(int ientry,int first_s,each_entry *sides)
{
	int		entry;
	each_entry	temprec;

	entry=ientry;

	while ((entry > first_s) &&
     		(sides[entry].x_int < sides[entry-1].x_int)) {
/*
C	swap(sides(entry),sides(entry-1))
C	again heavy use of aggregate assignment statments...
*/
		temprec = sides[entry];
		sides[entry] = sides[entry-1];
		sides[entry-1] = temprec;

		entry=entry-1;
	}

	return;
}
/*----------------------level 2 ------------------------------- */

void	draw_lines(each_entry *sides,int scan,int x_int_count,int iindex,
		int *list,int *ptr,int max_list)
{
	int	index;
	int	k,x1,x2;

	index=iindex;

	for(k=1;k<=((x_int_count)/2.0);k++) {

		while (sides[index].delta_y == 0) {
			index=index+1;
		}

		x1=(sides[index].x_int);
		index=index+1;

		while (sides[index].delta_y == 0) {
			index=index+1;
		}

		x2=(sides[index].x_int);
/*
C	place the line in the buffer if there is room.
C	If the buffer fills then return PTR=-1.
*/
		if ((*ptr) != -1) {
			*ptr=(*ptr)+3;
			if ((*ptr) <= max_list) {
				list[(*ptr)-1]=scan;
				list[(*ptr)-2]=x2;
				list[(*ptr)-3]=x1; 
			} else {
				*ptr=-1;
			}
		}
/*
C	next line segment.
*/
		index=index+1;
	}

	return;
}

/*----------------------level 2 -------------------------------*/

void	update_sides_list(each_entry *sides,int first_s,int last_s)
{
	int	k;
	
	for(k=first_s; k <= last_s;k++) {
/*	do with sides(k) */
		if (sides[k].delta_y > 0) {
			sides[k].delta_y=sides[k].delta_y-1;
			sides[k].x_int=sides[k].x_int-sides[k].x_change_per_scan;
		}
/*	enddo {with} */
	}	
	
	return;
}
